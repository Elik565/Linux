#pragma once

#include <dirent.h>
#include <sys/stat.h>

#define MAX_SIZE_PATH 128

struct archiver
{
    char path[MAX_SIZE_PATH];  // путь до архивируемого каталога

    DIR* dirp;  // указатель на каталог 

    char folder_name[MAX_SIZE_PATH];  // имя папки
};

struct file_info {
        char path[MAX_SIZE_PATH];

        size_t size;
};

///// ПРОТОТИПЫ ФУНКЦИЙ /////

// Выбор каталога для архивирования
void choose_catalog(struct archiver* arch);

void find_folder_name(struct archiver* arch);

void dir_passage(char* dir);

