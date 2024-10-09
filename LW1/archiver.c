#include "archiver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


void choose_catalog(struct Archiver* arch) {
    while (arch->dirptr == NULL) {
        printf("Введите путь до папки, которую хотите заархивировать: ");
        scanf("%s", arch->dir_path);

        if (strcmp(arch->dir_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Программа завершена пользователем\n");
            return;
        }

        arch->dirptr = opendir(arch->dir_path);
        if (!arch->dirptr) {
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
        fprintf(stderr, "Путь слишком длинный!\n");
        return;
    }

    if (arch->archiv_path[strlen(arch->archiv_path)] != '/') {  // если на конце нет '/'
        strcat(arch->archiv_path, "/");  // добавляем '/' 
    }

    strcat(arch->archiv_path, arch->folder_name);
    strcat(arch->archiv_path, ".dat");
}

void add_to_archive(const struct file_info* info, struct Archiver* arch) {
    fprintf(arch->arch_file, "File path: %s, ", info->path);
    fprintf(arch->arch_file, "size: %d\n\n", info->size);
}

void dir_passage(char* dir, char* current_path, struct Archiver* arch) {
    DIR* dirptr;
    struct dirent* entry; 
    struct stat statbuf;

    size_t cur_path_len = strlen(current_path);  // длина текущего пути

    arch->arch_file = fopen(arch->archiv_path, "wb");  // открываем архивный файл
    if (arch->arch_file == NULL) {
        fprintf(stderr, "Ошибка открытия архива для записи!\n");
        return;
    }

    if ((dirptr = opendir(dir)) == NULL) {
        fprintf(stderr, "Не получается открыть директорию: %s\n", dir);
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

            // добавляем к текущему пути текущую директорию
            strcat(current_path, entry->d_name);
            strcat(current_path, "/");

            dir_passage(entry->d_name, current_path, arch);  // рекурсивный вызов 

            current_path[cur_path_len] = '\0';  // восстанавливаем текущий путь после рекурсии
        }
        else {
            struct file_info info;

            info.size = statbuf.st_size;  // размер файла

            // путь к файлу
            info.path[0] = '\0';
            strcat(info.path, current_path);
            strcat(info.path, entry->d_name);
            
            add_to_archive(&info, arch);  // добавляем информацию о файле в архив
        }
    }

    chdir("..");  // переходим в родительскую директорию
    closedir(dirptr);  // закрываем директорию
}
