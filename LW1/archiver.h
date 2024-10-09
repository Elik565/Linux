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

    DIR* dirptr;  // указатель на каталог 

    char folder_name[MAX_SIZE_PATH];  // имя папки

    FILE* arch_file;  // архивный файл

    struct file_info* files;  // информация о файлах директории

    size_t files_count;  // количество файлов в директории
};

// Структура информации о файле
struct file_info {
        char path[MAX_SIZE_PATH];

        char real_path[MAX_SIZE_PATH];

        size_t size;
};

///// ПРОТОТИПЫ ФУНКЦИЙ /////

// Выбор каталога для архивирования
void choose_catalog(struct Archiver* arch);

// Поиск имени директории
void find_folder_name(struct Archiver* arch);

// Выбор пути сохранения архива
void choose_arch_path(struct Archiver* arch);

// 
void add_to_archive(struct Archiver* arch);

// Сбор информации о файлах
void collect_files_info(const char* dir, char* current_path, struct Archiver* arch);

// Архивирование директории
void archive(struct Archiver* arch);

// Разархивирование директории
void unzip();

