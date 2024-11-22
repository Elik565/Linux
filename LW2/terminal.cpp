#include "terminal.hpp"
#include <dirent.h>
#include <fstream>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <filesystem>
#include <sys/resource.h>

namespace fs = std::filesystem;

pid_t child_pid = 0;

void handle_sigint(int sig) {
    if (child_pid > 0) {  // если запущен дочерний процесс
        killpg(child_pid, SIGKILL);
    }
    else {  // если нет дочернего процесса
        exit(0);  // завершаем родительский процесс
    }
    std::cout << "\n";
}

std::string separate_input(std::string& input, std::vector<std::string>& params) {
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

    input.erase(0, i + 1);  // очищаем ввод от команды

    // заполняем массив параметров
    i = 0;
    std::string param;
    while (i <= input.size()) {
        if (input[i] == ' ' || i == input.size()) {
            if (!param.empty()) {
                params.push_back(param);
                param.clear();
            }
        }
        else {
            param += input[i];
        }

        i++;
    }

    return command;
}

void cat_handler(const std::vector<char*>& params) {
    char buff[512];

    if (params.size() == 1) {  // елси нет параметров
        while (true) {
            std::cin >> buff;
            std::cout << buff << std::endl;
        }
    } 

    else {
        for (size_t i = 0; i < params.size() - 1; i++) {
            FILE* fin = fopen(params[i], "r");

            while (fgets(buff, sizeof(buff), fin)) { // Читаем построчно
                std::cout << buff;  // Выводим строку
            }

            fclose(fin);
            buff[0] = '\0';
        }
    }

    exit(0);
}

void ls_handler(const std::vector<char*>& params) {
    std::string param;
    if (params.size() == 1) {  // если нет параметров
        param = fs::current_path();

        size_t i = 0;
        for (const auto& entry : fs::directory_iterator(param)) {  // перебираем все файлы в текущей папке
            if (i == 4) {  // в строку выводим только 4 элемента
                std::cout << std::endl;
                i = 0;
            }
            std::cout << entry.path().filename().string() << " ";
            i++;
        }
    }

    else {  // если есть параметры
        for (size_t i = 0; i < params.size() - 1; i++) {
            param = params[i];

            if (fs::exists(param) && fs::is_directory(param)) {  // если путь существует и это папка
                std::cout << param << ":\n";
                size_t i = 0;
                for (const auto& entry : fs::directory_iterator(param)) {  // перебираем все файлы в папке
                    if (i == 4) {  // в строку выводим только 4 элемента
                        std::cout << "\n";
                        i = 0;
                    }
                    std::cout << entry.path().filename().string() << " ";
                    i++;
                }
                std::cout << std::endl;
            }

            else if (fs::exists(param)) {  // если путь существует и это не папка
                std::cout << param;
            }

            else {
                std::cout << "Нет такого файла или директории";
            }

            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
    exit(0);
}

void nice_handler(const std::vector<char*>& params) {
    int niceness = 10;  // значение приоритета по умолчанию

    if (params.size() == 1)  // если нет параметров
        std::cout << '0' << std::endl;

    else {
        if (std::string(params[0]) == "-n") {  // если есть флаг
            if (params.size() == 2 or params.size() == 3) {
                std::cout << "Недостаточно аргументов для команды nice" << std::endl;
                exit(0);
            }
            
            // проверка значения приоритета
            niceness = std::stoi(params[1]);
            if (niceness < -20) {
                niceness = -20;
            }
            else if (niceness > 19) {
                niceness = 19;
            }
        }

        
    }

    std::cout << std::endl;
    exit(0);
}

void start_process(const std::string& command, const std::vector<char*>& params) {
    child_pid = fork();  // создаем дочерний процесс

    if (child_pid == 0) {  // если мы в дочернем процессе
        setpgid(0, 0);  // создаем группу процессов
        prctl(PR_SET_PDEATHSIG, SIGKILL);  // если завершится родительский процесс, то завершаем и дочерний

        if (command.empty()) {
            exit(0);
        }

        else if (command == "cat") {
            cat_handler(params);
        }

        else if (command == "ls") {
            ls_handler(params);
        }

        else if (command == "nice") {
            nice_handler(params);
        }
        
        else {
            execvp(command.c_str(), params.data());  // запуск команды
        }
    } 
    else if (child_pid > 0) {
        setpgid(child_pid, child_pid);  // устанавливаем группу для дочернего процесса
        int status;

        waitpid(child_pid, &status, 0);  // ожидаем завершения дочернего процесса
        child_pid = 0;  // сбрасываем PID после завершения
    } 
    else {
        std::cerr << "Ошибка создания дочернего процесса\n";
    }
}

