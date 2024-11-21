#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Sobel.hpp"
#include "stb_image-master/stb_image.h"
#include "stb_image-master/stb_image_write.h"
#include <vector>
#include <pthread.h>
#include <iostream>

int main() {
    std::cout << "a;skdja;kg";
    int width, height, channels;
    std::vector<int> threads = {1, 2, 4, 8, 16, 32};  // массив кол-во потоков
    
    unsigned char* input_img = stbi_load("mai.png", &width, &height, &channels, 1);  // загрузка изображения

    unsigned char* output_img = new unsigned char[width * height * channels];  // изображение на выходе

    // основной алгортитм
    for (size_t i = 0; i < threads.size(); i++) {  // проходимся по всем количествам потоков
        size_t count_threads = threads[i];  // кол-во потоков

        size_t rows = (height - 1) / count_threads;

        size_t start_row = 1;
        size_t end_row = rows;

        pthread_t threads_list[threads[count_threads]];  // массив потоков

        for (size_t j = 0; j < count_threads; j++) {  // проходимся по всем потокам
            pthread_t thread;  // поток

            // заполняем струткуру данных потока
            ThreadData thread_data;
            thread_data.start_row = start_row;
            thread_data.end_row = end_row;
            thread_data.height = height;
            thread_data.width = width;
            thread_data.input_img = input_img;
            thread_data.output_img = output_img;

            pthread_create(&thread, nullptr, applySobelFilter, &thread_data);

            // обновляем строки
            start_row = end_row ;
            end_row += rows;
        }

        for(size_t j = 0; j < threads[i]; j++)
		    pthread_join(threads_list[j], nullptr);
    }

    stbi_write_png("output.png", width, height, 1, output_img, width);

    stbi_image_free(input_img);
    return 0;
}
