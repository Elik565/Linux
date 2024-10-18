#include "archiver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // очищаем буфер ввода
    }
}

int test_path(const char* path) {
    int path_exists = 1;
    char temp_path[MAX_SIZE_PATH] = "";
    size_t i = 0;
    size_t k = 0;
    if (path[0] == '/') {
        i++;
        k++;
        temp_path[0] = '/';
    }

    size_t len_path = strlen(path);

    while (i < len_path) {
        while (path[i] != '/' && i < len_path) {
            temp_path[k] = path[i];
            i++;
            k++;
        }

        struct stat statbuf;
        if (lstat (temp_path, &statbuf) == -1) {
            if (mkdir(temp_path, 0755) == -1) {
                fprintf(stderr, "Ошибка в пути %s\n\n", path);
                return 1;
            }
        }
        temp_path[k++] = '/';
        i++;
    }

    return 0;
}

int choose_dir(char* dir_path) {
    DIR* dir_ptr = NULL;

    while (dir_ptr == NULL) {
        printf("Введите путь до директории, которую хотите заархивировать: ");
        fgets(dir_path, MAX_SIZE_PATH, stdin);
        dir_path[strcspn(dir_path, "\n")] = '\0';

        if (strcmp(dir_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Архивация завершена пользователем\n\n");
            return 1;
        }

        if (strlen(dir_path) + 2 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
            clear_input_buffer();
        }
        else {
            dir_ptr = opendir(dir_path);
            if (!dir_ptr) {
                printf("Директория не найдена!\n");
            }
        }
    }

    closedir(dir_ptr);
    return 0;
}

void find_folder_name(struct Archive* arch) {
    size_t path_size = strlen(arch->dir_path);
    if (arch->dir_path[path_size - 1] == '/') {  // проверяем есть ли в конце '/'
        path_size--;
    }
    
    int i = path_size - 1;
    size_t len_name = 0;
    while (arch->dir_path[i] != '/' && i >= 0) {  // находим размер названия директории 
        len_name++;
        i--;
    }
    
    size_t j = 0;
    for (int i = len_name; i > 0; i--) {  // записываем название директории в переменную
        arch->folder_name[j] = arch->dir_path[path_size - i];
        j++;
    }
}

int choose_arch_path(struct Archive* arch) {
    int cond = 1;

    while (cond == 1) {
        printf("Введите путь до директории, где сохранить архив: ");
        fgets(arch->archiv_path, sizeof(arch->archiv_path), stdin);
        arch->archiv_path[strcspn(arch->archiv_path, "\n")] = '\0';

        if (strcmp(arch->archiv_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Архивация завершена пользователем\n\n");
            return 1;
        }

        if (strlen(arch->archiv_path) + 2 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
            clear_input_buffer();
        }

        else if (test_path(arch->archiv_path) == 0) {
            if (arch->archiv_path[strlen(arch->archiv_path)] != '/') {  // если на конце нет '/'
                strcat(arch->archiv_path, "/");  // добавляем '/' 
            }

            find_folder_name(arch);

            strcat(arch->archiv_path, arch->folder_name);
            strcat(arch->archiv_path, ".dat");

            cond = 0;
        }

        if (cond == 0) {
            arch->arch_file = fopen(arch->archiv_path, "wb");  // оставляем открытым файл для записи
            if (arch->arch_file == NULL) {
                fprintf(stderr, "Ошибка открытия архива для записи!\n");
                cond = 1;
            }
        }
    }

    return 0;
}

int collect_files_info(const char* dir, char* current_path, struct Archive* arch) {
    DIR* dirptr;
    struct dirent* entry; 
    struct stat statbuf;

    size_t cur_path_len = strlen(current_path);  // длина текущего пути

    if ((dirptr = opendir(dir)) == NULL) {
        fprintf(stderr, "Не получается открыть директорию: %s\n", dir);
        return 1;
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
            arch->files = realloc(arch->files, (arch->files_count + 1) * sizeof(struct file_info));  // выделяем память под новый файл
    
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

    return 0;
}

void add_header_to_archive(struct Archive* arch) {
    fprintf(arch->arch_file, "*****Header*****\n\n");
    fprintf(arch->arch_file, "The number of files in the archive: %d\n\n", arch->files_count);
    for (size_t i = 0; i < arch->files_count; i++) {
        fprintf(arch->arch_file, "Path: %s\nSize: %zu\n", arch->files[i].path, arch->files[i].size);
    }
    fprintf(arch->arch_file, "\n*****Files data*****\n\n");
}

void arithmetic_coding(void* memory_ptr) {
    double low = 0;
    double high = 1;


}

int add_data_to_archive(struct Archive* arch) {
    for (size_t i = 0; i < arch->files_count; i++) {
        FILE* fin = fopen(arch->files[i].real_path, "rb");

        if (fin == NULL) {
            fprintf(stderr, "Ошибка открытия файла %s для архивации!\n", arch->files[i].real_path);
            return 1;
        }

        void* memory_ptr = malloc(arch->files[i].size);  // динамически выделяем память под данные файла

        fread(memory_ptr, 1, arch->files[i].size, fin);  // читаем данные из файла

        arithmetic_coding(memory_ptr);
        
        fwrite(memory_ptr, 1, arch->files[i].size, arch->arch_file);  // записываем данные в архив

        free(memory_ptr);
        fclose(fin);
    }

    fclose(arch->arch_file);

    return 0;
}

void archive() {
    // создание структуры архивации
    struct Archive arch;
    arch.files = malloc(sizeof(struct file_info));
    arch.files_count = 0;
    arch.archiv_path[0] = '\0';
    arch.dir_path[0] = '\0';
    
    // создание текущего пути для записи обхода директории
    char current_path[MAX_SIZE_PATH];
    current_path[0] = '\0';

    // выбор пути директории для архивирования
    if (choose_dir(arch.dir_path) != 0) {
        return;
    }

    // выбор пути, по которому создастся архив
    if (choose_arch_path(&arch) != 0) {
        return;
    }

    // сбор инфо о файлах
    if (collect_files_info(arch.dir_path, current_path, &arch) != 0) {
        return;
    }  

    // запись заголовка в архив
    add_header_to_archive(&arch);

    // запись данных в архив
    if (add_data_to_archive(&arch) != 0) {
        return;
    }  

    printf("Архив успешно создан!\n\n");
}




int choose_file_path(struct Extract* extr) {
    extr->arch_file = NULL;

    while (extr->arch_file == NULL) {
        printf("Введите путь до архива: ");
        fgets(extr->archiv_path, sizeof(extr->archiv_path), stdin);
        extr->archiv_path[strcspn(extr->archiv_path, "\n")] = '\0';

        if (strcmp(extr->archiv_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Разархивация завершена пользователем\n\n");
            return 1;
        }

        if (strlen(extr->archiv_path) + 1 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
            clear_input_buffer();
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

    return 0;
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

int choose_extract_path(struct Extract* extr) {
    int cond = 1;

    while (cond == 1) {
        printf("Введите путь, куда разархивировать файл: ");
        fgets(extr->extract_path, sizeof(extr->extract_path), stdin);
        extr->extract_path[strcspn(extr->extract_path, "\n")] = '\0';

        if (strcmp(extr->extract_path, "q") == 0) {  // если нажали 'q', то выходим из программы
            printf("Разархивация завершена пользователем\n\n");
            return 1;
        }

        if (strlen(extr->extract_path) + 2 >= MAX_SIZE_PATH) {
            fprintf(stderr, "Путь слишком длинный!\n");
            clear_input_buffer();
        }
        else {
            cond = test_path(extr->extract_path);
        }

        if (cond == 0) {
            if (extr->extract_path[strlen(extr->extract_path) - 1] != '/') {  // если на конце нет '/'
                strcat(extr->extract_path, "/");  // добавляем '/' 
            }

            strcat(extr->extract_path, extr->arch_name);

            // Создание директории из архива
            if (mkdir(extr->extract_path, 0755) == -1) {
                fprintf(stderr, "Не удалось создать директорию по пути %s\n\n", extr->extract_path);
                cond = 1;
            }
        }
    }

    return 0;
}

void separate_file_name(char* path) {
    size_t i = strlen(path);
    while (path[i] != '/' && i > 0) {
        i--;
    }

    path[i] = '\0';
}

void read_header(struct Extract* extr) {
    fscanf(extr->arch_file, "*****Header*****\n\nThe number of files in the archive: %zu\n", &extr->files_count);  // считываем кол-во файлов 

    extr->files = malloc(extr->files_count * sizeof(struct file_info));  // динамически выделяем память под файлы

    // считываем инфо о файлах 
    char line[MAX_SIZE_PATH];
    for (size_t i = 0; i < extr->files_count; i++) {
        // считываем путь 
        fgets(line, sizeof(line), extr->arch_file);
        sscanf(line, "Path: %[^\n]", extr->files[i].path);

        // считываем размер
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

int extract_data(struct Extract* extr) {
    chdir(extr->extract_path);  // переходим в разархивированную директорию 

    // проходимся по всем данным архива
    for (size_t i = 0; i < extr->files_count; i++) {
        strcat(extr->files[i].real_path, extr->files[i].path);
        separate_file_name(extr->files[i].path);

        test_path(extr->files[i].path);  // проверка на несуществующие директории

        FILE* fout = fopen(extr->files[i].real_path, "wb");  // открываем файл для записи

        if (fout == NULL) {
            fprintf(stderr, "Ошибка создания файла %s для записи!\n", extr->files[i].real_path);
            return 1;
        }

        void* memory_ptr = malloc(extr->files[i].size);  // динамически выделяем память под данные файла

        fread(memory_ptr, 1, extr->files[i].size, extr->arch_file);  // читаем данные файла из архива

        fwrite(memory_ptr, 1, extr->files[i].size, fout);  // записываем данные в файл

        fclose(fout);
        free(memory_ptr);
    }

    chdir("..");
    fclose(extr->arch_file);

    return 0;
}

void extract() {
    // создание структуры разархивации
    struct Extract extr;  
    extr.archiv_path[0] = '\0';
    extr.extract_path[0] = '\0';

    // выбор пути к архиву
    if (choose_file_path(&extr) != 0) {
        return;
    }

    find_arch_name(&extr);  // поиск имени архива
    
    // выбор пути для разархивирования
    if (choose_extract_path(&extr) != 0) {
        return;
    }

    read_header(&extr);  // чтение заголовка из архива

    // извлечение данных из архива
    if (extract_data(&extr) != 0) {
        return;
    }

    printf("Разархивация прошла успешно!\n\n");
} 