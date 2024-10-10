#include <stdio.h>
#include "archiver.h"
#include <dirent.h>
#include <string.h>


int main() {
    printf("АРХИВАТОР\n\n");

    while (1) {
        printf("Для архивации директории нажмите '1';\n");
        printf("Для разархивации файла нажмите '2';\n");
        printf("Для выхода из программы нажмите 'q';\n");

        char choise[5];
        while (strcmp(choise, "1") != 0 && strcmp(choise, "2") != 0) {
            printf("Ввод: ");
            scanf("%s", choise);

            if (strcmp(choise, "q") == 0) {
                printf("Программа завершена пользователем\n");
                return 0;
            }
        }

        if (strcmp(choise, "1") == 0) {  // если ввели 1
            archive();
        }

        if (strcmp(choise, "2") == 0) {  // если ввели 2

        }

        choise[0] = '\0';
    }
    

    
    return 0;
}