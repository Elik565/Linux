#pragma once

#include <iostream>
#include <string>

// Отделение команды от параметров
std::string separate_input(std::string& input);

// Запуск процессов, указанных во входной строке 
void start_process(std::string& command, std::string& params);