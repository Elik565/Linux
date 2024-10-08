#include <stdio.h>
#include "archiver.h"
#include <dirent.h>
#include <string.h>


int main() {
    // Создание структуры архиватора
    struct archiver arch;
    arch.dirp = NULL;
    
    // Выбор каталога для архивирования
    choose_catalog(&arch);
    if (arch.dirp == NULL) {
        return 1;
    }

    // нахождение имени папки
    find_folder_name(&arch);

    dir_passage(arch.path);
    


    return 0;
}