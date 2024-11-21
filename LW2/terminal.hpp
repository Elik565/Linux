#pragma once

#include <iostream>
#include <string>
#include <vector>

extern pid_t child_pid;

// Отделение команды от параметров
std::string separate_input(std::string& input, std::vector<std::string>& params);

// Обработчик прерывания
void handle_sigint(int sig);

// Запуск процессов не по умолчанию
void start_not_default_process(const std::string& command, const std::vector<char*>& params);

// Обработчик команды cat
void cat_handler(const std::vector<char*>& params);

// Обработчик команд по умолчанию
void default_command_handler(const std::string& command, const std::vector<char*>& params);

// Запуск процессов по умолчанию
void start_process(const std::string& command, const std::vector<char*>& params);
