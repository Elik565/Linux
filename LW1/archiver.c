#include "archiver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

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

int bml(int num, int pos, int base) {
    return (num << pos) % base;
}

int bmlt(int num, int pos, int base) {
    num <<= pos;
    for (int i = 0; i < pos; i++) {
        num = num | (1 << i);
    }

    return num % base;
}

int bits_plus_follow(int bit, int bits_to_follow) {
    int code = bit;

    while (bits_to_follow > 0) {
        bits_to_follow--;
        code = (code << 1) | bit;
    }

    return code;
}

double round_to_decimal(double num, int decimal_places) {
    double factor = pow(10, decimal_places);
    return round(num * factor);
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

    arch->folder_name[len_name] = '\0';
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

void calc_freq_table(const unsigned char* memory_ptr, double freq_table[MAX_COUNT_SYMBOLS][2]) {
    int length = strlen((const char*)memory_ptr);
    double high = 1.0;
    double possible;
    
    int counts[MAX_COUNT_SYMBOLS] = {0};

    for (int i = 0; i < length; i++) {
        counts[(unsigned char)memory_ptr[i]]++;
    }

    for (int i = 0; i < MAX_COUNT_SYMBOLS; i++) {
        if (counts[i] > 0) {
            possible = (double)counts[i] / length;
            freq_table[i][0] = (high - possible) > 1e-14 ? round_to_decimal(high - possible, 14) : 0;
            freq_table[i][1] = round_to_decimal(high, 14);
            high -= possible;
        }
    }
}

size_t arithmetic_coding(const unsigned char* memory_ptr, double freq_table[MAX_COUNT_SYMBOLS][2], unsigned char* encoded_data) {
    int Low = 0;
    int High = MAX_SIZE;
    int bits_to_follow = 1;
    int index = 0;  // Индекс для записи в буфер encoded_data

    // Для каждого байта входного потока данных memory_ptr
    for (int i = 0; memory_ptr[i] != '\0'; i++) {
        unsigned char symbol = memory_ptr[i];

        // Вычисление границ Low и High на основе частот символов
        int newLow = Low + freq_table[symbol][0] * (High - Low + 1) / MAX_SIZE;
        int newHigh = Low + freq_table[symbol][1] * (High - Low + 1) / MAX_SIZE;
        Low = newLow;
        High = newHigh;

        while (1) {
            if (High < HALF) {  // Если старший бит 0
                encoded_data[index++] = '0';
                bits_to_follow = bits_plus_follow(0, bits_to_follow);
                Low = bml(Low, 1, MAX_SIZE + 1);
                High = bmlt(High, 1, MAX_SIZE + 1);
            } else if (Low >= HALF) {  // Если старший бит 1
                encoded_data[index++] = '1';
                bits_to_follow = bits_plus_follow(1, bits_to_follow);
                Low = bml(Low - HALF, 1, MAX_SIZE + 1);
                High = bmlt(High - HALF, 1, MAX_SIZE + 1);
            } else if (Low >= FIRST_QTR && High < THIRD_QTR) {  // Если старшие биты разные
                bits_to_follow++;
                Low -= FIRST_QTR;
                High -= FIRST_QTR;
                Low = bml(Low, 1, MAX_SIZE + 1);
                High = bmlt(High, 1, MAX_SIZE + 1);
            } else {
                break;
            }
        }
    }

    // Финальная обработка для Low
    bits_to_follow++;
    for (int j = 0; j < bits_to_follow; j++) {
        if (Low < HALF) {
            encoded_data[index++] = '0';
        } else {
            encoded_data[index++] = '1';
        }
        Low = bml(Low, 1, MAX_SIZE + 1);
    }

    return index;
}

int add_data_to_archive(struct Archive* arch) {
    // проходимся по всем файлам для записи в архив
    for (size_t i = 0; i < arch->files_count; i++) {
        FILE* fin = fopen(arch->files[i].real_path, "rb");

        if (fin == NULL) {
            fprintf(stderr, "Ошибка открытия файла %s для архивации!\n", arch->files[i].real_path);
            return 1;
        }

        unsigned char* memory_ptr = (unsigned char*)malloc(arch->files[i].size);  // динамически выделяем память под данные файла

        fread(memory_ptr, 1, arch->files[i].size, fin);  // читаем данные из файла
        memory_ptr[arch->files[i].size] = '\0';
        fclose(fin);

        // алгоритм арифметического кодирования
        double freq_table[MAX_COUNT_SYMBOLS][2] = {{0.0}};
        calc_freq_table(memory_ptr, freq_table);  // расчет таблицы вероятностей символов

        // записываем таблицу вероятностей в файл
        for (int i = 0; i < MAX_COUNT_SYMBOLS; i++) {
            fwrite(freq_table[i], sizeof(double), 2, arch->arch_file);
        }

        unsigned char* encoded_data = (unsigned char*)malloc(MAX_SIZE);
        size_t data_size = arithmetic_coding(memory_ptr, freq_table, encoded_data);  // кодирование данных
        
        for(int i = 0; i < data_size; i++) {
            printf("%d", encoded_data[i]);
        }

        fwrite(&data_size, sizeof(size_t), 1, arch->arch_file);  // записываем размер кодированных данных
        fwrite(encoded_data, 1, data_size, arch->arch_file);  // записываем закодированные данные

        // Освобождаем память
        free(memory_ptr);
        free(encoded_data);
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
    
    int i = path_size;
    size_t len_format = 0;
    while (extr->archiv_path[i] != '.') {  // находим длину формата архива (на случай содержания в имени точек)
        len_format++;
        i--;
    }
    
    size_t len_name = 0;
    i = path_size - len_format - 1;  // учитываем точку
    while (extr->archiv_path[i] != '/' && i >= 0) {  // находим размер названия архива
        len_name++;
        i--;
    }

    size_t j = 0;
    for (int i = len_name + len_format; i > len_format; i--) {  // записываем название архива в переменную
        extr->arch_name[j] = extr->archiv_path[path_size - i];
        j++;
    }

    extr->arch_name[len_name] = '\0';
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
        // считываем путь файла
        fgets(line, sizeof(line), extr->arch_file);
        sscanf(line, "Path: %[^\n]", extr->files[i].path);

        // считываем размер файла
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

unsigned char find_symbol(struct Symbol* symbols, const long double value) {
    for (size_t i = 0; i < MAX_COUNT_SYMBOLS; i++) {
        if (value >= symbols[i].low && value < symbols[i].high) {
            return (unsigned char)i;
        }       
    }

    return UCHAR_MAX;
}

void arithmetic_decoding(const unsigned char* memory_ptr, int freq_table[MAX_COUNT_SYMBOLS][2], int length, char* decoded_data) {
    int Low = 0;
    int High = MAX_SIZE;
    int index = 0;  // Индекс для чтения bit из memory_ptr
    int decoded_index = 0;

    // Преобразуем первые биты закодированной строки в число
    int value = 0;
    for (int i = 0; i < BIT_LIMIT; i++) {
        value = (value << 1) | (memory_ptr[index++] - '0');
    }

    while (decoded_index < length) {  // Декодируем все символы
        // Вычисляем диапазон и частотное значение
        int range = High - Low + 1;
        int freq_value = ((value - Low + 1) * MAX_SIZE - 1) / range;

        // Находим символ на основе freq_value
        unsigned char symbol;
        for (symbol = 0; symbol < MAX_COUNT_SYMBOLS; symbol++) {
            if (freq_value < freq_table[symbol][1]) {
                break;
            }
        }

        // Добавляем символ к декодированным данным
        decoded_data[decoded_index++] = symbol;

        // Обновляем границы Low и High
        int newLow = Low + freq_table[symbol][0] * range / MAX_SIZE;
        int newHigh = Low + freq_table[symbol][1] * range / MAX_SIZE;
        Low = newLow;
        High = newHigh;

        // Обрабатываем биты
        while (High < HALF || Low >= HALF) {
            if (High < HALF) {
                Low = 2 * Low;
                High = 2 * High + 1;
                if (index < length) {  // Проверяем, что не вышли за пределы данных
                    value = (value << 1) | (memory_ptr[index++] - '0');
                } else {
                    value = (value << 1);  // Если данные закончились, сдвигаем без нового бита
                }
            } else if (Low >= HALF) {
                Low = 2 * (Low - HALF);
                High = 2 * (High - HALF) + 1;
                if (index < length) {
                    value = (value << 1) | (memory_ptr[index++] - '0');
                } else {
                    value = (value << 1);
                }
            }
        }

        // Обрабатываем ситуацию, если Low и High попадают в третью четверть
        while (Low >= FIRST_QTR && High < THIRD_QTR) {
            Low = 2 * (Low - FIRST_QTR);
            High = 2 * (High - FIRST_QTR) + 1;
            if (index < length) {
                value = (value << 1) | (memory_ptr[index++] - '0');
            } else {
                value = (value << 1);
            }
        }
    }

    decoded_data[decoded_index] = '\0';  // Завершаем строку
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

        int freq_table[MAX_COUNT_SYMBOLS][2] = {{0}};
        fread(freq_table, sizeof(freq_table), 1, extr->arch_file);  // читаем вероятности символов из архива

        long double value;  // значение для декодирования
        fread(&value, sizeof(long double), 1, extr->arch_file);  // читаем значение для декодирования из архива

        unsigned char* memory_ptr = (unsigned char*)malloc(MAX_SIZE);  // динамически выделяем память под закодированные данные файла
        if (memory_ptr == NULL) {  // Проверка на успешное выделение памяти
            fprintf(stderr, "Ошибка выделения памяти для разархивации файла %s!\n", extr->files[i].real_path);
            fclose(fout);
            return 1; 
        }

        unsigned char* decoded_data = (unsigned char*)malloc(extr->files[i].size);
        arithmetic_decoding(memory_ptr, freq_table, strlen(memory_ptr), decoded_data);  // декодирование данных

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