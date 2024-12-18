#include <fstream>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <sys/stat.h>
#include <fcntl.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

volatile bool need_read = false;

void sigusr1_handler(int sig) {
    need_read = true;  // устанавливаем флаг, что данные в FIFO1 готовы
}

void read_and_output_file(const std::string& file_path, const std::string& fifo_path, int signal) {
    pid_t p_pid = getppid();  // получаем pid родителя
    pid_t cur_pid = getpid();  // получает pid текущего процесса

    std::ifstream fin(file_path);

    if (!fin) {
        std::cerr << "Ошибка при открытии файла для чтения в программе 1 с pid " << cur_pid << std::endl;
        kill(p_pid, SIGTERM);
        exit(1);
    }

    int fifo_fd;  // дескриптор канала FIFO
    fifo_fd = open(fifo_path.c_str(), O_WRONLY);  // открываем канал FIFO только для записи

    if (fifo_fd == -1) {
        std::cerr << "Ошибка при открытии канала FIFO в программе 1 с pid " << cur_pid << std::endl;
        kill(p_pid, SIGTERM);
        exit(1);
    }

    // записываем данные
    char buff[512];
    while (true) {
        while (!need_read) {
            pause();
        }

        std::cout << "Программа 1 с pid " << cur_pid << " получила сигнал\n";

        need_read = false;

        if (fin.read(buff, sizeof(buff)) || fin.gcount() > 0) {
            pthread_mutex_lock(&mutex);  // захват мьютекса (блокирование FIFO)
            write(fifo_fd, buff, fin.gcount());  // gcount() выдает кол-во символов, прочитанных последний раз
            pthread_mutex_unlock(&mutex);  // освобождаем мьютекс (FIFO)

            std::cout << "Программа с pid " << cur_pid << " отправила сигнал\n";
            kill(p_pid, signal);
        }
        else {
            std::cout << "В программе 1 с pid " << cur_pid << " закончился файл\n";
            kill(p_pid, signal);
            break;
        }
    }

    close(fifo_fd);  // закрываем канал FIFO
    fin.close();  // закрываем файл
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Неправильные входные парметры для программы 1\n";
        exit(1);
    }

    signal(SIGUSR1, sigusr1_handler);

    int signal = std::stoi(argv[3]);
    read_and_output_file(argv[1], argv[2], signal);

    return 0;
}