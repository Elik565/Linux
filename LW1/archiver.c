#include "archiver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void choose_dir(struct Archive* arch) {
    DIR* dir_ptr = NULL;

    while (dir_ptr == NULL) {
        printf("Введите путь до директории, которую хотите заархивировать: ");
        fgets(arch->dir_path, sizeof(arch->dir_path), stdin);
        arch->dir_path[strcspn(arch->dir_path, "\n")] = '\0';

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
            printf("Директория не найдена!\n");
        }
    }

    closedir(dir_ptr);
}

void find_folder_name(struct Archive* arch) {
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

void choose_arch_path(struct Archive* arch) {
    int OK = 0;

    while (OK != 1) {
        printf("Введите путь до директории, где сохранить архив: ");
        fgets(arch->archiv_path, sizeof(arch->archiv_path), stdin);
        arch->archiv_path[strcspn(arch->archiv_path, "\n")] = '\0';

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

    // создаем директорию, если пользователь ввел несуществующую
    DIR* dir_ptr = opendir(arch->archiv_path);
    if (dir_ptr == NULL) {
        if (mkdir(arch->archiv_path, 0755) == -1) {
            fprintf(stderr, "Не удалось создать архив по пути %s\n\n", arch->archiv_path);
            arch->exit = 1;
            return;
        }
    }

    if (arch->archiv_path[strlen(arch->archiv_path)] != '/') {  // если на конце нет '/'
        strcat(arch->archiv_path, "/");  // добавляем '/' 
    }

    strcat(arch->archiv_path, arch->folder_name);
    strcat(arch->archiv_path, ".dat");
}

void collect_files_info(const char* dir, char* current_path, struct Archive* arch) {
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
            arch->files = realloc(arch->files, (arch->files_count + 1) * sizeof(struct file_info));
    
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

void add_header_to_archive(struct Archive* arch) {
    arch->arch_file = fopen(arch->archiv_path, "wb");  // оставляем открытым файл для записи

    if (arch->arch_file == NULL) {
        fprintf(stderr, "Ошибка открытия архива для записи!\n");
        return;
    }

    fprintf(arch->arch_file, "*****Header*****\n\n");
    fprintf(arch->arch_file, "The number of files in the archive: %d\n\n", arch->files_count);
    for (size_t i = 0; i < arch->files_count; i++) {
        fprintf(arch->arch_file, "Path: %s\nSize: %zu\n", arch->files[i].path, arch->files[i].size);
    }
    fprintf(arch->arch_file, "\n*****Files data*****\n\n");
}

void add_data_to_archive(struct Archive* arch) {
    for (size_t i = 0; i < arch->files_count; i++) {
        FILE* fin = fopen(arch->files[i].real_path, "rb");

        if (fin == NULL) {
            fprintf(stderr, "Ошибка открытия файла %s для архивации!\n", arch->files[i].real_path);
            return;
        }

        void* memory_ptr = malloc(arch->files[i].size);  // динамически выделяем память под данные файла

        fread(memory_ptr, 1, arch->files[i].size, fin);  // читаем данные из файла
        
        fwrite(memory_ptr, 1, arch->files[i].size, arch->arch_file);  // записываем данные в архив

        free(memory_ptr);
        fclose(fin);
    }

    fclose(arch->arch_file);
}

void archive() {
    // Создание структуры архивации
    struct Archive arch;
    arch.files = NULL;
    arch.files_count = 0;
    arch.exit = 0;
    arch.archiv_path[0] = '\0';
    arch.dir_path[0] = '\0';
    
    // создание текущего пути для записи обхода директории
    char current_path[MAX_SIZE_PATH];
    current_path[0] = '\0';

    // выбор пути директории для архивирования
    choose_dir(&arch);  
    if (arch.exit == 1) {
        return;
    }

    find_folder_name(&arch);  // нахождение имени папки

    // выбор пути, по которому создастся архив
    choose_arch_path(&arch);  
    if (arch.exit == 1) {
        return;
    }

    collect_files_info(arch.dir_path, current_path, &arch);  // сбор инфо о файлах

    add_header_to_archive(&arch);  // запись заголовка в архив

    add_data_to_archive(&arch);  // запись данных в архив

    printf("Архив успешно создан!\n\n");
}




void choose_file_path(struct Extract* extr) {
    extr->arch_file = NULL;

    while (extr->arch_file == NULL) {
        printf("Введите путь до архива: ");
        fgets(extr->archiv_path, sizeof(extr->archiv_path), stdin);
        extr->archiv_path[strcspn(extr->archiv_path, "\n")] = '\0';

        if (strcmp(extr->archiv_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Разархивация завершена пользователем\n\n");
            extr->exit = 1;
            return;
        }

        if (strlen(extr->archiv_path) + 1 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
        }
        
        extr->arch_file = fopen(extr->archiv_path, "rb");  // оставляем файл открытым для чтения

        struct stat statbuf;
        lstat (extr->archiv_path, &statbuf);

        if (S_ISDIR(statbuf.st_mode)) {
            extr->arch_file = NULL;
        }

        if (extr->arch_file == NULL) {
            printf("Файл не найден!\n");
        }
    }
}

void find_arch_name(struct Extract* extr) {
    size_t path_size = strlen(extr->archiv_path) - 1;
    
    size_t i = path_size;
    size_t len_format = 0;
    while (extr->archiv_path[i] != '.') {  // находим длину формата архива (на случай содержания в имени точек)
        len_format++;
        i--;
    }
    
    size_t len_name = 0;
    i = path_size - len_format - 1;  // учитываем точку
    while (extr->archiv_path[i] != '/') {  // находим размер названия архива
        len_name++;
        i--;
    }

    size_t j = 0;
    for (int i = len_name + len_format; i > len_format; i--) {  // записываем название архива в переменную
        extr->arch_name[j] = extr->archiv_path[path_size - i];
        j++;
    }
}

void choose_extract_path(struct Extract* extr) {
    int OK = 0;

    while (OK != 1) {
        printf("Введите путь, куда разархивировать файл: ");
        fgets(extr->extract_path, sizeof(extr->extract_path), stdin);
        extr->extract_path[strcspn(extr->extract_path, "\n")] = '\0';

        if (strcmp(extr->extract_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Разархивация завершена пользователем\n\n");
            extr->exit = 1;
            return;
        }

        if (strlen(extr->extract_path) + 2 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
        }
        else {
            OK = 1;
        }
    }

    if (extr->extract_path[strlen(extr->extract_path) - 1] != '/') {  // если на конце нет '/'
        strcat(extr->extract_path, "/");  // добавляем '/' 
    }

    // создаем директорию, если пользователь ввел несуществующую
    DIR* dir_ptr = opendir(extr->extract_path);
    if (dir_ptr == NULL) {
        if (mkdir(extr->extract_path, 0755) == -1) {
            fprintf(stderr, "Не удалось создать архив по пути %s\n\n", extr->extract_path);
            extr->exit = 1;
            return;
        }
    }

    strcat(extr->extract_path, extr->arch_name);

    // Создание директории из архива
    if (mkdir(extr->extract_path, 0755) == -1) {
        fprintf(stderr, "Не удалось создать директорию по пути %s\n\n", extr->extract_path);
        extr->exit = 1;
        return;
    }
}

void read_header(struct Extract* extr) {
    fscanf(extr->arch_file, "*****Header*****\n\nThe number of files in the archive: %zu\n", &extr->files_count);  // считываем кол-во файлов 

    extr->files = malloc(extr->files_count * sizeof(struct file_info));  // динамически выделяем память под файлы

    // считываем инфо о файлах 
    char line[MAX_SIZE_PATH];
    for (size_t i = 0; i < extr->files_count; i++) {
        fgets(line, sizeof(line), extr->arch_file);
        sscanf(line, "Path: %[^\n]", extr->files[i].path);

        fgets(line, sizeof(line), extr->arch_file);
        sscanf(line, "Size: %zu", &extr->files[i].size);
    }

    // читаем пока не дойдем до данных файлов
    char buff[128] = "";
    while (strcmp(buff, "*****Files data*****") != 0) {
        fgets(buff, sizeof(buff), extr->arch_file);
        buff[strcspn(buff, "\n")] = '\0';
    }
    fgets(buff, sizeof(buff), extr->arch_file); 
}

void test_path(char* path) {
    size_t len_path = strlen(path);
    size_t dir_count = 0;

    // узнаем кол-во директорий в пути
    for (size_t i = 0; i < len_path; i++) {
        if (path[i] == '/') {
            dir_count++;
        }
    }

    size_t k = 0;
    size_t j = 0;
    char parent_dir[MAX_SIZE_NAME] = "";

    for (size_t i = 0; i < dir_count; i++) {  // пока есть вложенные директории
        char dir_name[MAX_SIZE_NAME] = "";
        
        while (j < len_path) {
            if (path[j] != '/') {
                dir_name[k] = path[j];
                j++;
                k++;
            }
            else {  // если нашли имя директории
                // записываем имя родительской директории
                strcat(parent_dir, "../");

                // проверяем существует ли данная директория
                struct stat statbuf;
                if (lstat(dir_name, &statbuf) == -1) {  // если директория не существует
                    // создаем директорию
                    if (mkdir(dir_name, 0755) == -1) {  
                        fprintf(stderr, "Не удалось создать вложенную директорию\n");
                        return;
                    }
                }

                chdir(dir_name);  // переходим в созданную директорию

                k = 0;
                j++;
                
                break;
            }
        }
    }

    chdir(parent_dir);  // переходим в родительскую директорию
}

void extract_data(struct Extract* extr) {
    chdir(extr->extract_path);  // переходим в разархивированную директорию 

    // проходимся по всем данным архива
    for (size_t i = 0; i < extr->files_count; i++) {
        test_path(extr->files[i].path);  // проверка на несуществующие директории

        FILE* fout = fopen(extr->files[i].path, "wb");  // открываем файл для записи

        if (fout == NULL) {
            fprintf(stderr, "Ошибка создания файла %s для записи!\n", extr->files[i].path);
            extr->exit = 1;
            return;
        }

        void* memory_ptr = malloc(extr->files[i].size);  // динамически выделяем память под данные файла

        fread(memory_ptr, 1, extr->files[i].size, extr->arch_file);  // читаем данные файла из архива

        fwrite(memory_ptr, 1, extr->files[i].size, fout);  // записываем данные в файл

        fclose(fout);
        free(memory_ptr);
    }

    chdir("..");
    fclose(extr->arch_file);
}

void extract() {
    // Создание структуры разархивации
    struct Extract extr;  
    extr.exit = 0;
    extr.archiv_path[0] = '\0';
    extr.extract_path[0] = '\0';

    // Выбор пути к архиву
    choose_file_path(&extr);
    if (extr.exit == 1) {
        return;
    }

    find_arch_name(&extr);  // поиск имени архива
    
    // Выбор пути для разархивирования
    choose_extract_path(&extr);
    if (extr.exit == 1) {
        return;
    }

    read_header(&extr);  // чтение заголовка из архива

    // Извлечение данных из архива
    extract_data(&extr);
    if (extr.exit == 1) {
        return;
    }

    printf("Разархивация прошла успешно!\n\n");
} 