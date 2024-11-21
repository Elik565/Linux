#pragma once

#include <vector>

// Структура изображения
struct ImageData {
    int width;  // ширина
    int height;  // высота 
    int channels;  // каналы
    unsigned char* image;  // изображение
};

// Структура данных потока
struct ThreadData {
    int start_row;  // начальная строка
    int end_row;  // конечная строка
    int width;
    int height;
    unsigned char* input_img;
    unsigned char* output_img;
};

// Применяем фильтр собеля к изображению
void* applySobelFilter(void* arg);

// Применяет фильтр Собеля к изображению с помощью многопоточности
unsigned char* main_algorithm(const std::vector<int>& threads, const ImageData& image_data);