#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

#define MAX_SIZE_PATH 128

// Структура архиватора
struct Archiver
{
    char dir_path[MAX_SIZE_PATH];  // путь до архивируемого каталога

    char archiv_path[MAX_SIZE_PATH];  // путь до архива

    char folder_name[MAX_SIZE_PATH];  // имя папки

    FILE* arch_file;  // архивный файл

    struct file_info* files;  // информация о файлах директории

    size_t files_count;  // количество файлов в директории

    char extract_path[MAX_SIZE_PATH];  // путь для извлечения

    int exit;  // на случай выхода
};

// Структура информации о файле
struct file_info {
        char path[MAX_SIZE_PATH];  // относительный путь 

        char real_path[MAX_SIZE_PATH];  // абсолютный путь

        size_t size;  // размер файла
};


///// ПРОТОТИПЫ ФУНКЦИЙ ДЛЯ АРХИВИРОВАНИЯ /////

// Выбор каталога для архивирования
void choose_catalog(struct Archiver* arch);

// Поиск имени директории
void find_folder_name(struct Archiver* arch);

// Выбор пути сохранения архива
void choose_arch_path(struct Archiver* arch);

// Сбор информации о файлах
void collect_files_info(const char* dir, char* current_path, struct Archiver* arch);

// Запись заголовка в архив
void add_header_to_archive(struct Archiver* arch);

// Запись данных в архив
void add_data_to_archive(struct Archiver* arch);

// Архивирование директории
void archive();


///// ПРОТОТИПЫ ФУНКЦИЙ ДЛЯ РАЗАРХИВИРОВАНИЯ /////

// Выбор пути разархивирования
void choose_extract_path(struct Archiver* arch);

// Разархивирование директории
void unzip();

