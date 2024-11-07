#include "terminal.hpp"

std::string separate_input(std::string& input) {
    int i = 0;

    while (input[i] == ' ') {  // пропускаем начальные пробелы
        i++;
    }

    // отделяем команду от параметров
    std::string command;
    while (input[i] != ' ' && i < input.size()) {
        command.push_back(input[i]);
        i++;
    }

    input.erase(0, i);

    return command;
}

void start_process(std::string& command, std::string& params) {
    std::string full_command = command + params;

    if (command == "ls" || command == "cat" || command == "nice" || command == "killall") {
        if ((command == "cat") && params.empty()) {
            std::cout << "Недостаточно аргументов для команды cat!\n";
            return;
        }

        std::system(full_command.c_str());
    }

    else {
        
    }
}