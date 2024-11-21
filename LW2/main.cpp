#include <iostream>
#include "terminal.hpp"
#include <csignal>
#include <sys/wait.h>

int main() {
    std::signal(SIGINT, handle_sigint);

    std::cout << "My_Terminal\n\n";

    std::string input;
    while (true) {
        std::cout << "Ввод: ";
        std::getline(std::cin, input);  // ввод строки

        if (input == "q") {
            std::cout << "Программа завершена пользователем\n";
            break;
        }

        // получаем команду и параметры
        std::vector<std::string> params;
        std::string command = separate_input(input, params);

        // переделываем строки в char*
        std::vector<char*> exec_params;
        for (auto str : params) {
            exec_params.push_back(const_cast<char*>(str.c_str()));
        }
        exec_params.push_back(nullptr);  // для окончания параметров
        
        start_process(command, exec_params);
    }

    return 0;
}