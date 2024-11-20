#pragma once

#include <iostream>
#include <string>

extern pid_t child_pid;

// Отделение команды от параметров
std::string separate_input(std::string& input);

// Обработчик прерывания
void handle_sigint(int sig);

// Запуск процессов, указанных во входной строке 
void start_process(std::string& command, std::string& params);
