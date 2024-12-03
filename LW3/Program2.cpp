#include <iostream>
#include <pthread.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

bool data_ready_fifo1 = false;
bool data_ready_fifo2 = false;

void sigusr1_handler(int sig) {
    data_ready_fifo1 = true;  // устанавливаем флаг, что данные в FIFO1 готовы
}

void sigusr2_handler(int sig) {
    data_ready_fifo2 = true;  // устанавливаем флаг, что данные в FIFO2 готовы
}

void xor_output_data(const std::string& fifo1_path, const std::string& fifo2_path, const std::string& output_file) {
    // открываем канал FIFO1 только для чтения
    int fifo1_fd = open(fifo1_path.c_str(), O_RDONLY);  // ждем появления данных в канале fifo1
    if (fifo1_fd == -1) {
        std::cerr << "Ошибка при открытии канала FIFO1 для чтения\n";
        exit(1);
    }

    // открываем канал FIFO2 только для чтения
    int fifo2_fd = open(fifo2_path.c_str(), O_RDONLY);  // ждем появления данных в канале fifo
    if (fifo2_fd == -1) {
        std::cerr << "Ошибка при открытии канала FIFO2 для чтения\n";
        exit(1);
    }

    std::ofstream fout(output_file);  // открываем файл для записи результата

    if (!fout) {
        std::cerr << "Ошибка при открытии файла для записи\n";
        exit(1);
    }

    char buff1[512], buff2[512];
    size_t readed_bytes1, readed_bytes2;
    size_t old_readed_bytes2;

    while (true) {
        while (!data_ready_fifo1 && !data_ready_fifo2) {
            pause();  // ждем сигнала
        }

        std::cout << "Программа 2 получила сигналы\n";
        
        pthread_mutex_lock(&mutex);  // захват мьютекса (блокирование каналов FIFO)

        readed_bytes1 = read(fifo1_fd, buff1, sizeof(buff1));

        readed_bytes2 = read(fifo2_fd, buff2, sizeof(buff2));   

        pthread_mutex_unlock(&mutex);  // освобождение мьютекса (каналов FIFO)

        if (readed_bytes1 == 0 and readed_bytes2 == 0) {  // если нет данных в обоих каналах
            break; 
        }

        if (readed_bytes2 == 0) {  // если во втором канале закончились данные, а в первом нет
            readed_bytes2 = old_readed_bytes2;
        }
        else {
            old_readed_bytes2 = readed_bytes2;
        }

        size_t bytes_to_process = std::max(readed_bytes1, readed_bytes2);

        // побитовая опепация xor с выходными данными
        size_t j = 0;
        for (size_t i = 0; i < bytes_to_process; i++) {
            if (j >= readed_bytes2) 
                j = 0;

            char xor_byte = buff1[i] ^ buff2[j];
            fout << xor_byte;
            readed_bytes1--;
            j++;
        }
    }

    // закрываем каналы
    close(fifo1_fd);
    close(fifo2_fd);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Неправильные входные парметры для программы 2\n";
        exit(1);
    }

    // устанавливаем обработчики сигналов
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);

    std::string fifo1_path = "/tmp/fifo1";  // первый канал FIFO
    std::string fifo2_path = "/tmp/fifo2";  // второй канал FIFO

    // создание каналов FIFO с правами 0666
    mkfifo(fifo1_path.c_str(), 0666);
    mkfifo(fifo2_path.c_str(), 0666);

    pid_t pid1 = fork();
    if (pid1 == 0) {  // если находимся в дочернем процессе
        // запускаем программу 1 с текстовым файлом
        execlp("./Program1", "./Program1", argv[1], fifo1_path.c_str(), std::to_string(SIGUSR1).c_str(), NULL);  
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {  // если находимся в дочернем процессе
        // запускаем программу 1 со случайной последовательностью
        execlp("./Program1", "./Program1", argv[2], fifo2_path.c_str(), std::to_string(SIGUSR2).c_str(), NULL);  
        exit(0);
    }

    xor_output_data(fifo1_path, fifo2_path, argv[3]);

    // удаление каналов из файловой системы
    unlink(fifo1_path.c_str());
    unlink(fifo2_path.c_str());

    std::cout << "Программа успешно завершена!\n";

    return 0;
}