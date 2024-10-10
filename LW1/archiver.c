#include "archiver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void choose_catalog(struct Archiver* arch) {
    DIR* dir_ptr = NULL;

    while (dir_ptr == NULL) {
        printf("Введите путь до папки, которую хотите заархивировать: ");
        scanf("%s", arch->dir_path);

        if (strcmp(arch->dir_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Архивация завершена пользователем\n\n");
            arch->exit = 1;
            return;
        }

        if (strlen(arch->dir_path) + 2 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
        }

        dir_ptr = opendir(arch->dir_path);
        if (!dir_ptr) {
            printf("Каталог не найден!\n");
            dir_ptr = NULL;
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
    int OK = 0;

    while (OK != 1) {
        printf("Введите путь, где сохранить архив: ");
        scanf("%s", arch->archiv_path);

        if (strcmp(arch->archiv_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Архивация завершена пользователем\n\n");
            arch->exit = 1;
            return;
        }

        if (strlen(arch->archiv_path) + 2 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
        }
        else {
            OK = 1;
        }
    }

    if (arch->archiv_path[strlen(arch->archiv_path)] != '/') {  // если на конце нет '/'
        strcat(arch->archiv_path, "/");  // добавляем '/' 
    }

    strcat(arch->archiv_path, arch->folder_name);
    strcat(arch->archiv_path, ".dat");
}

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

void add_header_to_archive(struct Archiver* arch) {
    arch->arch_file = fopen(arch->archiv_path, "wb");  // открываем архивный файл

    if (arch->arch_file == NULL) {
        fprintf(stderr, "Ошибка открытия архива для записи!\n");
        return;
    }

    fprintf(arch->arch_file, "*****Header*****\n\n");
    fprintf(arch->arch_file, "The number of files in the archive: %d\n\n", arch->files_count);
    for (size_t i = 0; i < arch->files_count; i++) {
        fprintf(arch->arch_file, "Path: %s, size: %d\n\n", arch->files[i].path, arch->files[i].size);
    }
    fprintf(arch->arch_file, "\n*****Files data*****\n\n");
}

void add_data_to_archive(struct Archiver* arch) {
    for (size_t i = 0; i < arch->files_count; i++) {
        FILE* fin = fopen(arch->files[i].real_path, "rb");

        if (fin == NULL) {
            fprintf(stderr, "Ошибка открытия файла %s для архивации!\n", arch->files[i].real_path);
            return;
        }

        void* memory_ptr = malloc(arch->files[i].size);  // динамически выделяем память под данные файла

        fread(memory_ptr, 1, arch->files[i].size, fin);  // читаем данные из файла

        // записываем данные в архив
        fwrite(memory_ptr, 1, arch->files[i].size, arch->arch_file);  
        fprintf(arch->arch_file, "\n\n");

        free(memory_ptr);
        fclose(fin);
    }

    fclose(arch->arch_file);
}

void archive() {
    // Создание структуры архиватора
    struct Archiver arch;
    arch.files = NULL;
    arch.files_count = 0;
    arch.exit = 0;
    
    // создание текущего пути для записи обхода директории
    char current_path[MAX_SIZE_PATH];
    current_path[0] = '\0';
    strcat(current_path, "./");

    choose_catalog(&arch);  // выбор каталога для архивирования

    if (arch.exit == 1) {
        return;
    }

    find_folder_name(&arch);  // нахождение имени папки

    choose_arch_path(&arch);  // выбор пути, по которому создастся архив

    if (arch.exit == 1) {
        return;
    }

    collect_files_info(arch.dir_path, current_path, &arch);  // сбор инфо о файлах

    add_header_to_archive(&arch);  // запись заголовка в архив

    add_data_to_archive(&arch);  // запись данных в архив

    printf("Архив успешно создан!\n\n");
}



void choose_extract_path(struct Archiver* arch) {
    int OK = 0;

    while (OK != 1) {
        printf("Введите путь, куда разархивировать архив: ");
        scanf("%s", arch->extract_path);

        if (strcmp(arch->extract_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Программа завершена пользователем\n\n");
            arch->exit = 1;
            return;
        }

        if (strlen(arch->extract_path) + 2 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
        }
        else {
            OK = 1;
        }
    }

    if (arch->extract_path[strlen(arch->archiv_path)] != '/') {  // если на конце нет '/'
        strcat(arch->archiv_path, "/");  // добавляем '/' 
    }

    strcat(arch->extract_path, arch->folder_name);
}