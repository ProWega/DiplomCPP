#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// Структура для хранения цвета пикселя в формате RGB
struct Pixel {
    uint8_t r, g, b;
};

// Функция для загрузки изображения
bool load_image(const std::string& image_path, std::vector<Pixel>& image_data, int& width, int& height, int& channels) {
    unsigned char* img = stbi_load(image_path.c_str(), &width, &height, &channels, 3);
    if (!img) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        return false;
    }

    image_data.resize(width * height);
    for (int i = 0; i < width * height; ++i) {
        image_data[i] = { img[i * 3], img[i * 3 + 1], img[i * 3 + 2] };
    }

    stbi_image_free(img);
    return true;
}

// Функция для сохранения изображения
bool save_image(const std::string& image_path, const std::vector<Pixel>& image_data, int width, int height) {
    std::vector<unsigned char> data(image_data.size() * 3);
    for (int i = 0; i < width * height; ++i) {
        data[i * 3] = image_data[i].r;
        data[i * 3 + 1] = image_data[i].g;
        data[i * 3 + 2] = image_data[i].b;
    }
    return stbi_write_png(image_path.c_str(), width, height, 3, data.data(), width * 3) != 0;
}

// Преобразование в градации серого
void convert_to_grayscale(std::vector<Pixel>& image_data, int width, int height) {
    for (int i = 0; i < width * height; ++i) {
        uint8_t gray = 0.299 * image_data[i].r + 0.587 * image_data[i].g + 0.114 * image_data[i].b;
        image_data[i].r = image_data[i].g = image_data[i].b = gray;
    }
}

// Применение Gaussian Blur (размытие по Гауссу)
void apply_gaussian_blur(std::vector<Pixel>& image_data, int width, int height, int kernel_size = 5, double sigma = 1.0) {
    const double PI = 3.14159265358979323846;

    // Генерация ядра Гаусса
    std::vector<double> kernel(kernel_size * kernel_size);
    int half_size = kernel_size / 2;
    double sum = 0.0;
    for (int y = -half_size; y <= half_size; ++y) {
        for (int x = -half_size; x <= half_size; ++x) {
            double value = exp(-(x * x + y * y) / (2 * sigma * sigma)) / (2 * PI * sigma * sigma);
            kernel[(y + half_size) * kernel_size + (x + half_size)] = value;
            sum += value;
        }
    }
    // Нормализуем ядро
    for (auto& value : kernel) {
        value /= sum;
    }

    // Применяем размытие
    std::vector<Pixel> result(image_data.size());
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double r = 0, g = 0, b = 0;
            for (int ky = -half_size; ky <= half_size; ++ky) {
                for (int kx = -half_size; kx <= half_size; ++kx) {
                    int px = std::min(std::max(x + kx, 0), width - 1);
                    int py = std::min(std::max(y + ky, 0), height - 1);
                    const Pixel& pixel = image_data[py * width + px];
                    double weight = kernel[(ky + half_size) * kernel_size + (kx + half_size)];
                    r += pixel.r * weight;
                    g += pixel.g * weight;
                    b += pixel.b * weight;
                }
            }
            result[y * width + x] = { static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b) };
        }
    }
    image_data = result;
}

// Применение фильтра Собеля для выделения границ
void apply_sobel_filter(const std::vector<Pixel>& image_data, std::vector<Pixel>& result, int width, int height) {
    std::vector<int> sobel_x = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    std::vector<int> sobel_y = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

    result.resize(image_data.size());
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int gx = 0, gy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int px = x + kx;
                    int py = y + ky;
                    int weight_x = sobel_x[(ky + 1) * 3 + (kx + 1)];
                    int weight_y = sobel_y[(ky + 1) * 3 + (kx + 1)];
                    gx += weight_x * image_data[py * width + px].r;
                    gy += weight_y * image_data[py * width + px].r;
                }
            }
            int magnitude = std::sqrt(gx * gx + gy * gy);
            magnitude = std::min(255, std::max(0, magnitude));
            result[y * width + x] = { static_cast<uint8_t>(magnitude), static_cast<uint8_t>(magnitude), static_cast<uint8_t>(magnitude) };
        }
    }
}

// Основная функция
int main() {
    std::string input_image = "example2.jpg";
    std::string output_image = "output2.png";

    std::vector<Pixel> image_data;
    int width, height, channels;

    // Загрузка изображения
    if (!load_image(input_image, image_data, width, height, channels)) {
        return 1;
    }

    // Преобразование в градации серого
    convert_to_grayscale(image_data, width, height);

    // Применение Gaussian Blur
    apply_gaussian_blur(image_data, width, height, 5, 1.0);

    // Применение фильтра Собеля
    std::vector<Pixel> sobel_image;
    apply_sobel_filter(image_data, sobel_image, width, height);

    // Сохранение результата
    if (!save_image(output_image, sobel_image, width, height)) {
        std::cerr << "Failed to save image: " << output_image << std::endl;
        return 1;
    }

    std::cout << "Processing completed and saved as " << output_image << std::endl;
    return 0;
}