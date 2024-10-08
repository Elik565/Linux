#include <stdio.h>
#include "archiver.h"
#include <dirent.h>
#include <string.h>


int main() {
    // Создание структуры архиватора
    struct Archiver arch;
    arch.dirp = NULL;
    
    // Выбор каталога для архивирования
    choose_catalog(&arch);
    if (arch.dirp == NULL) {
        return 1;
    }

    // Нахождение имени папки
    find_folder_name(&arch);

    // Выбор пути, по которому создастся архив
    choose_arch_path(&arch);
    
    FILE* arhive = fopen(arch.archiv_path, "wb");

    dir_passage(arch.dir_path);
    


    return 0;
}