#include "archiver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


void choose_catalog(struct Archiver* arch) {
    while (arch->dirp == NULL) {
        printf("Введите путь до папки, которую хотите заархивировать: ");
        scanf("%s", arch->dir_path);

        if (strcmp(arch->dir_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Программа завершена пользователем\n");
            return;
        }

        arch->dirp = opendir(arch->dir_path);
        if (!arch->dirp) {
            printf("Каталог не найден!\n");
        }
    }
}

void find_folder_name(struct Archiver* arch) {
    size_t path_size = strlen(arch->dir_path) - 1;
    if (arch->dir_path[path_size] == '/') {  // проверяем есть ли в конце '/'
        path_size--;
    }
    
    size_t i = path_size;
    size_t len_name = 0;
    while (arch->dir_path[i] != '/') {  // находим размер названия директории 
        len_name++;
        i--;
    }
    
    size_t j = 0;
    for (int i = len_name - 1; i >= 0; i--) {  // записываем название директории в переменную
        arch->folder_name[j] = arch->dir_path[path_size - i];
        j++;
    }
}

void choose_arch_path(struct Archiver* arch) {
    printf("Введите путь, где сохранить архив: ");
    scanf("%s", arch->archiv_path);

    if (strcmp(arch->dir_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Программа завершена пользователем\n");
            return;
        }

    if (strlen(arch->archiv_path) + 1 >= MAX_SIZE_PATH) {
        fprintf(stderr, "путь слишком длинный!\n");
        return;
    }

    if (arch->archiv_path[strlen(arch->archiv_path)] != '/') {  // если на конце нет '/'
        strcat(arch->archiv_path, "/");  // добавляем '/' 
    }

    strcat(arch->archiv_path, arch->folder_name);
    strcat(arch->archiv_path, ".dat");
}

void dir_passage(char* dir) {
    DIR* dirptr;
    struct dirent* entry; 
    struct stat statbuf;

    if ((dirptr = opendir(dir)) == NULL) {
        fprintf(stderr, "не получается открыть директорию: %s\n", dir);
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
