#include "terminal.hpp"
#include <dirent.h>
#include <fstream>
#include <sys/wait.h>
#include <sys/prctl.h>

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

    if (params.size() == 1) {
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

