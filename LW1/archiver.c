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
        lstat (entry->d_name, &statbuf);

        if (S_ISDIR(statbuf.st_mode)) {  // если это директория
            if (strcmp(".", entry->d_name) == 0 ||  // пропускаем текущую и родительскую директории
                strcmp("..", entry->d_name) == 0) {
                
                continue;
            }

            printf("%s/\n", entry->d_name);
            dir_passage(entry->d_name);  // рекурсивный вызов 
        }
        else {
            printf("%s\n", entry->d_name);
        }
    }

    chdir("..");  // переходим в родительскую директорию
    closedir(dirptr);  // закрываем директорию
}
