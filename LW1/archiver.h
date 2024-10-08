#pragma once

#include <dirent.h>
#include <sys/stat.h>

#define MAX_SIZE_PATH 128

struct Archiver
{
    char dir_path[MAX_SIZE_PATH];  // путь до архивируемого каталога

    char archiv_path[MAX_SIZE_PATH];  // путь до архива

    DIR* dirp;  // указатель на каталог 

    char folder_name[MAX_SIZE_PATH];  // имя папки
};

struct file_info {
        char file_path[MAX_SIZE_PATH];

        size_t size;
};

///// ПРОТОТИПЫ ФУНКЦИЙ /////

// Выбор каталога для архивирования
void choose_catalog(struct Archiver* arch);

void find_folder_name(struct Archiver* arch);

void choose_arch_path(struct Archiver* arch);

void dir_passage(char* dir);

