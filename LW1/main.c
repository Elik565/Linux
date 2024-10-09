#include <stdio.h>
#include "archiver.h"
#include <dirent.h>
#include <string.h>


int main() {
    // Создание структуры архиватора
    struct Archiver arch;
    arch.dirptr = NULL;
    
    // Выбор каталога для архивирования
    choose_catalog(&arch);
    if (arch.dirptr == NULL) {
        return 1;
    }

    // Нахождение имени папки
    find_folder_name(&arch);

    // Выбор пути, по которому создастся архив
    choose_arch_path(&arch);

    // создание текущего пути для записи обхода директории
    char current_path[MAX_SIZE_PATH];
    current_path[0] = '\0';
    strcat(current_path, "./");

    dir_passage(arch.dir_path, current_path, &arch);  // обход директории

    return 0;
}