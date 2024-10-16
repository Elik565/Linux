#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

#define MAX_SIZE_PATH 64
#define MAX_SIZE_NAME 64
#define MAX_COUNT_SYMBOLS 256


// Структура информации о файле
struct file_info {
    char path[MAX_SIZE_PATH];  // относительный путь 

    char real_path[MAX_SIZE_PATH];  // абсолютный путь

    size_t size;  // размер файла
};

// Структура архивации
struct Archive {
    char dir_path[MAX_SIZE_PATH];  // путь до архивируемого каталога

    char archiv_path[MAX_SIZE_PATH];  // путь до архива

    char folder_name[MAX_SIZE_NAME];  // имя директории

    FILE* arch_file;  // архивный файл

    struct file_info* files;  // информация о файлах директории

    size_t files_count;  // количество файлов в директории
};

// Структура разархивации
struct Extract {
    char archiv_path[MAX_SIZE_PATH];  // путь до архива

    char extract_path[MAX_SIZE_PATH];  // путь для извлечения

    char arch_name[MAX_SIZE_NAME];  // имя архива

    FILE* arch_file;  // файл архива

    struct file_info* files;  // информация о файлах в архиве

    size_t files_count;  // количество файлов в архиве
};

// Структура символа для арифметического кодирования
struct Symbol {
    double low;  // нижняя граница интервала

    double high;  // верхняя граница интервала
};


///// ПРОТОТИПЫ ВСПОМОГАТЕЛЬНЫХ ФУНКЦИЙ /////

// Очистка буфера ввода
void clear_input_buffer();

// Проверка пути на наличие несуществующих директорий
int test_path(const char* path);


///// ПРОТОТИПЫ ФУНКЦИЙ ДЛЯ АРХИВИРОВАНИЯ /////

// Выбор каталога для архивирования
int choose_dir(char* dir_path);

// Поиск имени директории
void find_folder_name(struct Archive* arch);

// Выбор пути сохранения архива
int choose_arch_path(struct Archive* arch);

// Сбор информации о файлах
int collect_files_info(const char* dir, char* current_path, struct Archive* arch);

// Запись заголовка в архив
void add_header_to_archive(struct Archive* arch);

// Расчет вероятностей символов
void calc_symbols_probabilities(char* memory_ptr, size_t file_size, struct Symbol* symbols);

// Арифметическое кодирование данных
void arithmetic_coding(char* memory_ptr, size_t file_size);

// Запись данных в архив
int add_data_to_archive(struct Archive* arch);

// Архивирование директории
void archive();


///// ПРОТОТИПЫ ФУНКЦИЙ ДЛЯ РАЗАРХИВИРОВАНИЯ /////

// Выбор пути к архиву
int choose_file_path(struct Extract* extr);

// Поиск имени архива
void find_arch_name(struct Extract* extr);

// Выбор пути разархивирования
int choose_extract_path(struct Extract* extr);

// Отделение имени файла от директорий
void separate_file_name(char* path);

// Чтение заголовка из архива
void read_header(struct Extract* extr);

// Извлечение данных из архива
int extract_data(struct Extract* extr);

// Разархивирование файла
void extract();

