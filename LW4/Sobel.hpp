#pragma once

struct ThreadData {
    int start_row;
    int end_row;
    int width;
    int height;
    unsigned char* input_img;
    unsigned char* output_img;
};

void* applySobelFilter(void* arg);