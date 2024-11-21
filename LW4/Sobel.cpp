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

unsigned char* main_algorithm(const std::vector<int>& threads, const ImageData& image_data) {
    unsigned char* output_img = new unsigned char[image_data.width * image_data.height * image_data.channels];  // изображение на выходе

    // основной алгортитм
    for (size_t i = 0; i < threads.size(); i++) {  // проходимся по всем количествам потоков
        size_t count_threads = threads[i];  // кол-во потоков

        size_t rows = (image_data.height - 1) / count_threads;

        size_t start_row = 1;
        size_t end_row = rows;

        pthread_t threads_list[count_threads];  // массив потоков

        ThreadData threads_data[count_threads];  // массив данных потоков

        for (size_t j = 0; j < count_threads; j++) {  // проходимся по всем потокам
            // заполняем струткуру данных потока
            threads_data[j].start_row = start_row;
            threads_data[j].end_row = end_row;
            threads_data[j].height = image_data.height;
            threads_data[j].width = image_data.width;
            threads_data[j].input_img = image_data.image;
            threads_data[j].output_img = output_img;

            pthread_create(&threads_list[j], nullptr, applySobelFilter, &threads_data[j]);  // создаем процесс

            // обновляем строки
            start_row = end_row ;
            end_row += rows;
        }

        // ожидание завершения всех потоков (блокируется основной поток)
        for(size_t j = 0; j < threads[i]; j++)
		    pthread_join(threads_list[j], nullptr);
    }

    return output_img;
}
