#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Sobel.hpp"
#include "stb_image-master/stb_image.h"
#include "stb_image-master/stb_image_write.h"
#include <vector>

int main() {
    ImageData image_data;  // структура изображения

    std::vector<int> threads = {1, 2, 4, 8, 16, 32};  // массив кол-ва потоков
    
    image_data.image = stbi_load("mai.png", &image_data.width, &image_data.height, &image_data.channels, 1);  // загрузка изображения

    unsigned char* output_img = main_algorithm(threads, image_data);  // применяем основной алгоритм

    stbi_write_png("output.png", image_data.width, image_data.height, 1, output_img, image_data.width);  // создаем выходное изображение

    stbi_image_free(image_data.image);  // очищаем входное изображение

    delete[] output_img;

    return 0;
}
