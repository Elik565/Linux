#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

#define MAX_SIZE_PATH 128

struct Archiver
{
    char dir_path[MAX_SIZE_PATH];  // путь до архивируемого каталога

    char archiv_path[MAX_SIZE_PATH];  // путь до архива

    DIR* dirptr;  // указатель на каталог 

    char folder_name[MAX_SIZE_PATH];  // имя папки

    FILE* arch_file;  // архивный файл
};

struct file_info {
        char path[MAX_SIZE_PATH];

        char real_path[MAX_SIZE_PATH];

        size_t size;
};

///// ПРОТОТИПЫ ФУНКЦИЙ /////

// Выбор каталога для архивирования
void choose_catalog(struct Archiver* arch);

void find_folder_name(struct Archiver* arch);

void choose_arch_path(struct Archiver* arch);

void add_to_archive(const struct file_info* info, struct Archiver* arch);

void dir_passage(char* dir, char* current_path, struct Archiver* arch);

