#include <fstream>
#include <iostream>

void read_and_output_file(const std::string& file_path, const std::string& fifo_path) {
    std::ifstream fin(FILE);

    if (!fin) {
        std::cout << "Ошибка при открытии файла\n";
    }

    
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Неправильные входные парметры для программы 1\n";
        exit(1);
    }

    read_and_output_file(argv[1], argv[2]);

    return 0;
}