#include <fstream>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <sys/stat.h>
#include <fcntl.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void read_and_output_file(const std::string& file_path, const std::string& fifo_path, int signal) {
    std::ifstream fin(file_path);

    if (!fin) {
        std::cerr << "Ошибка при открытии файла для чтения\n";
        exit(1);
    }

    int fifo_fd;  // дескриптор канала FIFO
    fifo_fd = open(fifo_path.c_str(), O_WRONLY);  // открываем канал FIFO только для записи

    if (fifo_fd == -1) {
        std::cerr << "Ошибка при открытии канала FIFO для записи\n";
        exit(1);
    }

    // записываем данные
    char buff[512];
    pid_t p_pid = getppid();  // получаем pid родителя
    pid_t cur_pid = getpid();  // получает pid текущего процесса
    while (fin.read(buff, sizeof(buff)) || fin.gcount() > 0) {
        pthread_mutex_lock(&mutex);  // захват мьютекса (блокирование FIFO)
        write(fifo_fd, buff, fin.gcount());  // gcount() выдает кол-во символов, прочитанных последний раз
        pthread_mutex_unlock(&mutex);  // освобождаем мьютекс (FIFO)

        std::cout << "Программа с pid " << cur_pid << " отправила сигнал\n";
        kill(p_pid, signal);
    }

    close(fifo_fd);  // закрываем канал FIFO
    fin.close();  // закрываем файл
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Неправильные входные парметры для программы 1\n";
        exit(1);
    }

    int signal = std::stoi(argv[3]);
    read_and_output_file(argv[1], argv[2], signal);

    return 0;
}