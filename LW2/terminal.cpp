#include "terminal.hpp"
#include <dirent.h>
#include <fstream>
#include <sys/wait.h>
#include <sys/prctl.h>

pid_t child_pid = 0;

void handle_sigint(int sig) {
    if (child_pid > 0) {
        killpg(child_pid, SIGKILL);
    }
    std::cout << "\n";
}

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

    input.erase(0, i + 1);

    return command;
}

void start_process(std::string& command, std::string& params) {
    child_pid = fork();  // создаем дочерний процесс

    if (child_pid == 0) {
        setpgid(0, 0);  // создаем группу процессов
        prctl(PR_SET_PDEATHSIG, SIGKILL);

        if (params.empty()) {
            execlp(command.c_str(), command.c_str(), nullptr);
        }

        else {
            execlp(command.c_str(), command.c_str(), params.c_str(), nullptr);
        }
        
        std::cerr << "Ошибка выполнения команды: " << command << "\n";
        exit(1);
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

