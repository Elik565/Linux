#include "archiver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void choose_catalog(struct archiver* arch) {
    while (arch->dirp == NULL) {
        printf("Введите путь до папки, которую хотите заархивировать: ");
        scanf("%s", arch->path);

        if (strcmp(arch->path, "q") == 0) {
            return;
        }

        arch->dirp = opendir(arch->path);
        if (!arch->dirp) {
            printf("Каталог не найден!\n");
        }
    }
}

void find_folder_name(struct archiver* arch) {
    size_t path_size = strlen(arch->path) - 1;
    if (arch->path[path_size] == '/') {  // проверяем есть ли в конце '/'
        path_size--;
    }
    
    size_t i = path_size;
    size_t len_name = 0;
    while (arch->path[i] != '/') {
        len_name++;
        i--;
    }
    
    size_t j = 0;
    for (int i = len_name - 1; i >= 0; i--) {
        arch->folder_name[j] = arch->path[path_size - i];
        j++;
    }
}

void dir_passage(char* dir) {
    DIR* dirptr;
    struct dirent* entry;
    struct stat statbuf;

    if ((dirptr = opendir(dir)) == NULL) {
        fprintf(stderr, "cannot open directory: %s\n", dir);
        return;
    }

    chdir(dir);  // меняет текущий каталог на переданный в функцию
    while ((entry = readdir(dirptr)) != NULL) {  // пока в каталоге есть файлы или директории
        lstat (entry->d_name, &statbuf);  // получаем информацию о файле или директории

        if (S_ISDIR(statbuf.st_mode)) {  // если это директория
            if (strcmp(".", entry->d_name) == 0 ||  // пропускаем текущую и родительскую директории
                strcmp("..", entry->d_name) == 0) {
                
                continue;
            }

            dir_passage(entry->d_name);  // рекурсивный вызов 
        }
        else {
            struct file_info info;
        }
    }

    chdir("..");  // переходим в родительскую директорию
    closedir(dirptr);  // закрываем директорию
}
