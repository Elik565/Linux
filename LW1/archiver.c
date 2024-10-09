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

        if (strlen(arch->archiv_path) + 1 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
            return;
        }

        arch->dirptr = opendir(arch->dir_path);
        if (!arch->dirptr) {
            printf("Каталог не найден!\n");
            return;
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

// void add_to_archive(struct Archiver* arch) {
//     fprintf(arch->arch_file, "File path: %s, ", path);
//     fprintf(arch->arch_file, "size: %d\n", info->size);

    
//     FILE* fin = fopen(info->real_path, "rb");
//     if (fin == NULL) {
//         fprintf(stderr, "Ошибка открытия файла %s для архивации!\n", info->real_path);
//         return;
//     }

//     void* memory_ptr = malloc(info->size);  // динамически выделяем память 

//     fread(memory_ptr, 1, info->size, fin);  // читаем данные из файла
//     fwrite(memory_ptr, 1, info->size, arch->arch_file);  // записываем данные в архив
//     fprintf(arch->arch_file, "\n\n");
// }

void collect_files_info(const char* dir, char* current_path, struct Archiver* arch) {
    DIR* dirptr;
    struct dirent* entry; 
    struct stat statbuf;

    size_t cur_path_len = strlen(current_path);  // длина текущего пути

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

            collect_files_info(entry->d_name, current_path, arch);  // рекурсивный вызов 

            current_path[cur_path_len] = '\0';  // восстанавливаем текущий путь после рекурсии
        }
        else {
            // выделяем память под новый файл
            struct file_info* temp = realloc(arch->files, (arch->files_count + 1) * sizeof(struct file_info));
            if (temp == NULL) {
                fprintf(stderr, "Ошибка выделения памяти!\n");
                return;
            }
            arch->files = temp;

            arch->files[arch->files_count].size = statbuf.st_size;  // размер файла

            // относительный путь к файлу
            arch->files[arch->files_count].path[0] = '\0';
            strcat(arch->files[arch->files_count].path, current_path);
            strcat(arch->files[arch->files_count].path, entry->d_name);

            // реальный путь к файлу
            arch->files[arch->files_count].real_path[0] = '\0';
            strcat(arch->files[arch->files_count].real_path, realpath(entry->d_name, NULL));
            
            arch->files_count++;  // увеличиваем кол-во файлов
        }
    }
    
    chdir("..");  // переходим в родительскую директорию
    closedir(dirptr);  // закрываем директорию
}

void archive(struct Archiver* arch) {
    arch->arch_file = fopen(arch->archiv_path, "wb");  // открываем архивный файл
    if (arch->arch_file == NULL) {
        fprintf(stderr, "Ошибка открытия архива для записи!\n");
        return;
    }
}
