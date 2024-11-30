#include <fstream>
#include <random>
#include <iostream>
#include <sys/stat.h>
#include <ctime>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Неправильные входные параметры для программы заполнения случайной последовательности\n";
        exit(1);
    }

    struct stat file_stat;

    if (stat(argv[1], &file_stat) != 0) {
        std::cerr << "Файл с текстом не найден\n";
        exit(1);
    }

    std::ofstream fout(argv[2]);  // открываем файл со случайной последовательностью

    if (!fout) {
        std::cerr << "Файл со случайной последовательностью не найден\n";
        exit(1);
    }

    std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<> dis(0, 256);

    for (size_t i = 0; i < file_stat.st_size; i++) {
        fout << static_cast<char>(dis(gen));
    }

    std::cout << "Файл со случайной последовательностью успешно заполнен!\n";

    return 0;
}