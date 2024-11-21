#include "Sobel.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <pthread.h>

// Функция для применения фильтра Собела к изображению
void* applySobelFilter(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int width = data->width;
    int height = data->height;
    int gx, gy;
    
    // Ядро фильтра Собела по оси X
    int kernelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    
    // Ядро фильтра Собела по оси Y
    int kernelY[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };
    
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 1; j < data->width - 1; j++) {
            gx = 0;
            gy = 0;
            
            // Применяем ядра фильтра Собела к пикселям изображения
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int pixel = data->input_img[(k + i) * width + (l + j)];
                    gx += pixel * kernelX[k + 1][l + 1];
                    gy += pixel * kernelY[k + 1][l + 1];
                }
            }
            
            // Вычисляем градиент изображения
            double gradient = sqrt(gx * gx + gy * gy);
            if (gradient > 255) gradient = 255;
            if (gradient < 0) gradient = 0;
            data->output_img[i * data->width + j] = (unsigned char)gradient;
        }
    }

    pthread_exit(nullptr);
}
