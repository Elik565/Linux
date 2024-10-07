#pragma once

#include <dirent.h>
#include <sys/stat.h>

struct archiver
{
    char path[128];  // путь до архивируемого каталога

    DIR* dirp;  // указатель на каталог 
};

///// ПРОТОТИПЫ ФУНКЦИЙ /////

// Выбор каталога для архивирования
void choose_catalog(struct archiver* arch);

void dir_passage(char* dir);
// 