#include "terminal.hpp"
#include <fstream>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <filesystem>
#include <sys/resource.h>
#include <algorithm>
#include <iostream>

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

    if (params.size() == 2) {  // если нет параметров
        while (fgets(buff, sizeof(buff), stdin)) {  // Читаем построчно
            std::cout << buff;  // Выводим строку
        }
    } 

    else {
        for (size_t i = 1; i < params.size() - 1; i++) {
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
    if (params.size() == 2) {  // если нет параметров
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
        for (size_t i = 1; i < params.size() - 1; i++) {
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
                std::cout << "Файл или директория не найдены";
            }

            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
    exit(0);
}

void nice_handler(const std::vector<char*>& params) {
    int niceness = 10;  // значение приоритета по умолчанию
    bool n_flag = false;  // флаг наличия -n

    if (params.size() == 2)  // если нет параметров
        std::cout << '0' << std::endl;

    else {
        if (std::string(params[1]) == "-n") {  // если есть флаг
            n_flag = true;

            if (params.size() == 3) {
                std::cout << "Нет значения приоритета" << std::endl;
                exit(1);
            }
            
            // проверка значения приоритета
            niceness = std::stoi(params[2]);
            if (niceness < -20) {
                niceness = -20;
            }
            else if (niceness > 19) {
                niceness = 19;
            }
        }

        child_pid = fork();

        if (child_pid == 0) {  // если мы в дочернем процессе
            setpgid(0, 0);  // создаем группу процессов
            
            // заполняем команду и параметры для запускаемого процесса
            size_t i = 1;
            if (n_flag) {
                if (params.size() < 5) {
                    std::cout << "Недостаточно параметров для команды nice" << std::endl;
                    exit(1);
                }

                i = 3;
            }

            if (params.size() < 3) {
                std::cout << "Недостаточно параметров для команды nice" << std::endl;
                exit(1);
            }

            std::string child_command = std::string(params[i]);
            std::vector<char*> child_params(params.begin() + i, params.end()); 

            if (child_command != "firefox") {  // firefox не создает дочерних процессов
                prctl(PR_SET_PDEATHSIG, SIGKILL);  // если завершится родительский процесс, то завершаем и дочерний
            }

            setpriority(PRIO_PROCESS, 0, niceness);  // устанавливаем приоритет

            execvp(child_command.c_str(), child_params.data());
        }

        else if (child_pid > 0) {  // если мы в родительском процессе
            setpgid(child_pid, child_pid);  // устанавливаем группу для дочернего процесса
            int status;
            waitpid(child_pid, &status, 0);  // ожидаем завершения дочернего процесса
            child_pid = 0;  // сбрасываем PID после завершения
        }
    }

    std::cout << std::endl;
    exit(0);
}

void killall_handler(const std::vector<char*>& params) {
    if (params.size() == 2) {  // если нет параметров
        std::cout << "Недостаточно параметров для команды killall" << std::endl;
        exit(1);
    }

    for (size_t i = 1; i < params.size() - 1; i++) {  // проходимся по всем параметрам
        const std::string proc_path = "/proc";  // путь к папке /proc

        for (const auto& entry : fs::directory_iterator(proc_path)) {  // проходимся по папке /proc
            // проверяем, что это директория
            if (!entry.is_directory())
                continue;

            // проверяем, что имя директории - это число
            std::string dir_name = entry.path().filename();
            if (!std::all_of(dir_name.begin(), dir_name.end(), ::isdigit))
                continue;

            pid_t pid = std::stoi(dir_name);
            
            std::string exe_file = entry.path().string() + "/exe";

            char exe_path[4096];
            ssize_t len = readlink(exe_file.c_str(), exe_path, sizeof(exe_path) - 1);
            if (len != -1) {
                exe_path[len] = '\0';
                std::string process_name = std::string(exe_path);

                // Сравниваем с полным путем
                if (process_name.find(params[i]) != std::string::npos) {
                    if (kill(pid, SIGTERM) != 0) {
                        std::cerr << "Не удалось завершить процесс " << pid << "\n";
                    }
                }
            }
        }
    }

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
        
        else if (command == "killall") {
            killall_handler(params);
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

