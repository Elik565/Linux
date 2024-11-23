#include "Program1.hpp"
#include <fstream>
#include <iostream>

void read_file(const char* FILE) {
    std::ifstream fin(FILE);

    if (!fin) {
        std::cout << "Файл не найден\n";
    }
}