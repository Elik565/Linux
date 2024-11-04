#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

#define MAX_SIZE_PATH 64
#define MAX_SIZE_NAME 64
#define MAX_COUNT_SYMBOLS 256

#define BIT_LIMIT 16  // разрядность числа
#define BASE 2  // система счисления
#define MAX_SIZE 65535  // максимальное число
#define BIT_MOVE 1  // сдвиг битов

#define FIRST_QTR 16384  // четверть
#define HALF 32768  // половина
#define THIRD_QTR 49152  // 3 четверти


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

    char** internal_arch_paths;  // пути до внутренних архивов

    size_t int_arch_count;  // количество внутренних архивов

    size_t int_files_count;  // количество файлов во внутренних архивах
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

// Проверка на внутренний архив
int check_for_archive(char* path);

// Сбор информации о файлах
int collect_files_info(const char* dir, char* current_path, struct Archive* arch);

// Добавление количества внутренних файлов архива во внешний
int calc_count_files(struct Archive* arch);

// Добавление файлов внутреннего архива во внешний
int arсhive_processing(struct Archive* arch);

// Запись заголовка в архив
int add_header_to_archive(struct Archive* arch);

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

