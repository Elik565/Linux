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

        std::string command = separate_input(input);

        start_process(command, input);
     }

    return 0;
}