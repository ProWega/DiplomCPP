#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>

// Структура для RGB пикселя
struct RGB {
    unsigned char r, g, b;
};

// Структура для компоненты
struct Component {
    std::vector<std::pair<int, int>> pixels;
};

// Функция для чтения изображения
std::vector<RGB> load_image(const std::string& image_path, int& width, int& height) {
    int channels;
    unsigned char* data = stbi_load(image_path.c_str(), &width, &height, &channels, 3);
    if (!data) {
        std::cerr << "Ошибка: не удалось загрузить изображение!" << std::endl;
        exit(1);
    }

    std::vector<RGB> image;
    image.reserve(width * height);

    for (int i = 0; i < width * height * 3; i += 3) {
        image.push_back({ data[i], data[i + 1], data[i + 2] });
    }

    stbi_image_free(data);
    return image;
}

// Сохранение изображения в формате JPG
void save_image_jpg(const std::string& filename, const std::vector<RGB>& image, int width, int height) {
    std::vector<unsigned char> image_data;
    image_data.reserve(width * height * 3);
    for (const auto& pixel : image) {
        image_data.push_back(pixel.r);
        image_data.push_back(pixel.g);
        image_data.push_back(pixel.b);
    }
    stbi_write_jpg(filename.c_str(), width, height, 3, image_data.data(), 100);
}

// Функция для кластеризации K-Means
/*
void kmeans(const std::vector<RGB>& pixels, std::vector<int>& labels, std::vector<RGB>& centers, int n_clusters) {
    std::vector<int> counts(n_clusters, 0);
    labels.resize(pixels.size());

    // Инициализация центров кластеров случайными пикселями
    centers.resize(n_clusters);
    for (int i = 0; i < n_clusters; ++i) {
        centers[i] = pixels[rand() % pixels.size()];
    }

    bool changed;
    do {
        changed = false;
        // Присваивание меток пикселям на основе минимального расстояния до центров
        for (size_t i = 0; i < pixels.size(); ++i) {
            double min_dist = std::sqrt(std::pow(pixels[i].r - centers[0].r, 2) +
                std::pow(pixels[i].g - centers[0].g, 2) +
                std::pow(pixels[i].b - centers[0].b, 2));
            int min_index = 0;

            for (int j = 1; j < n_clusters; ++j) {
                double dist = std::sqrt(std::pow(pixels[i].r - centers[j].r, 2) +
                    std::pow(pixels[i].g - centers[j].g, 2) +
                    std::pow(pixels[i].b - centers[j].b, 2));
                if (dist < min_dist) {
                    min_dist = dist;
                    min_index = j;
                }
            }

            if (labels[i] != min_index) {
                labels[i] = min_index;
                changed = true;
            }
        }

        // Пересчет центров кластеров
        std::fill(centers.begin(), centers.end(), RGB{ 0, 0, 0 });
        std::fill(counts.begin(), counts.end(), 0);

        for (size_t i = 0; i < pixels.size(); ++i) {
            centers[labels[i]].r += pixels[i].r;
            centers[labels[i]].g += pixels[i].g;
            centers[labels[i]].b += pixels[i].b;
            counts[labels[i]]++;
        }

        for (int j = 0; j < n_clusters; ++j) {
            if (counts[j] > 0) {
                centers[j].r /= counts[j];
                centers[j].g /= counts[j];
                centers[j].b /= counts[j];
            }
        }
    } while (changed);
}
*/
// Реализация K-Means кластеризации с исправлением
void kmeans(const std::vector<RGB>& pixels, std::vector<int>& labels, std::vector<RGB>& centers, int n_clusters) {
    std::vector<int> counts(n_clusters, 0);
    labels.resize(pixels.size());

    // Инициализация центров кластеров случайными пикселями
    centers.resize(n_clusters);
    for (int i = 0; i < n_clusters; ++i) {
        centers[i] = pixels[rand() % pixels.size()];
    }

    bool changed;
    do {
        changed = false;
        // Шаг 1: Присваивание меток пикселям на основе минимального расстояния до центров
        for (size_t i = 0; i < pixels.size(); ++i) {
            double min_dist = std::sqrt(std::pow(pixels[i].r - centers[0].r, 2) +
                std::pow(pixels[i].g - centers[0].g, 2) +
                std::pow(pixels[i].b - centers[0].b, 2));
            int min_index = 0;

            for (int j = 1; j < n_clusters; ++j) {
                double dist = std::sqrt(std::pow(pixels[i].r - centers[j].r, 2) +
                    std::pow(pixels[i].g - centers[j].g, 2) +
                    std::pow(pixels[i].b - centers[j].b, 2));
                if (dist < min_dist) {
                    min_dist = dist;
                    min_index = j;
                }
            }

            if (labels[i] != min_index) {
                labels[i] = min_index;
                changed = true;
            }
        }

        // Шаг 2: Пересчет центров кластеров
        std::fill(centers.begin(), centers.end(), RGB{ 0, 0, 0 });
        std::fill(counts.begin(), counts.end(), 0);

        // Используем временные переменные для накопления суммы цветов
        std::vector<int> sum_r(n_clusters, 0);
        std::vector<int> sum_g(n_clusters, 0);
        std::vector<int> sum_b(n_clusters, 0);

        // Считаем сумму значений цветов для каждого кластера
        for (size_t i = 0; i < pixels.size(); ++i) {
            int cluster_id = labels[i];
            sum_r[cluster_id] += pixels[i].r;
            sum_g[cluster_id] += pixels[i].g;
            sum_b[cluster_id] += pixels[i].b;
            counts[cluster_id]++;
        }

        // Обновляем центры кластеров
        for (int j = 0; j < n_clusters; ++j) {
            if (counts[j] > 0) {
                centers[j].r = sum_r[j] / counts[j];
                centers[j].g = sum_g[j] / counts[j];
                centers[j].b = sum_b[j] / counts[j];
            }
            else {
                // Если кластер пуст, назначаем ему случайный пиксель
                centers[j] = pixels[rand() % pixels.size()];
            }
        }

    } while (changed);
}


// DFS для поиска компонент
void dfs(int x, int y, int cluster_index, const std::vector<std::vector<int>>& clustered_pixels,
    std::vector<std::vector<bool>>& visited, Component& component, int width, int height) {
    std::vector<std::pair<int, int>> stack = { {x, y} };

    while (!stack.empty()) {
        auto [cx, cy] = stack.back();
        stack.pop_back();

        if (cx < 0 || cy < 0 || cx >= width || cy >= height || visited[cy][cx] || clustered_pixels[cy][cx] != cluster_index) {
            continue;
        }

        visited[cy][cx] = true;
        component.pixels.push_back({ cx, cy });

        // Добавляем соседей
        stack.push_back({ cx - 1, cy });
        stack.push_back({ cx + 1, cy });
        stack.push_back({ cx, cy - 1 });
        stack.push_back({ cx, cy + 1 });
    }
}

// Поиск замкнутых компонент
std::vector<Component> find_connected_components(const std::vector<std::vector<int>>& clustered_pixels,
    int cluster_index, int min_size, int width, int height) {
    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::vector<Component> components;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!visited[y][x] && clustered_pixels[y][x] == cluster_index) {
                Component component;
                dfs(x, y, cluster_index, clustered_pixels, visited, component, width, height);

                if (component.pixels.size() >= min_size) {
                    components.push_back(component);
                }
            }
        }
    }

    return components;
}

// Сортировка компонент (слева направо, сверху вниз)
std::vector<Component> sort_components(const std::vector<Component>& components) {
    auto top_left = [](const Component& component) {
        return *std::min_element(component.pixels.begin(), component.pixels.end(),
            [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                return std::tie(a.second, a.first) < std::tie(b.second, b.first);
            });
        };

    std::vector<Component> sorted_components = components;
    std::sort(sorted_components.begin(), sorted_components.end(),
        [&](const Component& a, const Component& b) {
            return top_left(a) < top_left(b);
        });

    return sorted_components;
}

// Выделение компонент
std::vector<RGB> highlight_components(const std::vector<RGB>& image, const Component& component, int width, int height) {
    std::vector<RGB> result(image.size(), { 255, 255, 255 }); // Белый фон
    for (const auto& pixel : component.pixels) {
        int idx = pixel.second * width + pixel.first;
        result[idx] = image[idx];
    }
    return result;
}
// Фильтрация фоновых пикселей, сохраняем их индексы
std::vector<int> filter_background(const std::vector<RGB>& pixels, std::vector<RGB>& filtered_pixels) {
    std::vector<int> indices;  // Индексы отфильтрованных пикселей в исходном изображении
    for (size_t i = 0; i < pixels.size(); ++i) {
        const auto& pixel = pixels[i];
        // Проверяем, что пиксель не белый или близок к белому (фон)
        if (!(pixel.r > 240 && pixel.g > 240 && pixel.b > 240)) {
            filtered_pixels.push_back(pixel);
            indices.push_back(i);  // Сохраняем индекс пикселя
        }
    }
    return indices;
}

// Основная функция
void main_process(const std::string& image_path, int n_clusters = 5, int min_component_size = 100) {
    int width, height;
    std::vector<RGB> image = load_image(image_path, width, height);

    std::cout << "Фильтрация фоновых пикселей " << std::endl;
    // Фильтрация фоновых пикселей
    std::vector<RGB> filtered_image;
    std::vector<int> filtered_indices = filter_background(image, filtered_image);
    std::cout << "Пиксели отфильтрованы " << std::endl;
    // Кластеризация изображения (на основе отфильтрованных пикселей)
    std::vector<int> labels;
    std::vector<RGB> centers;
    std::cout << "Кластеризация начата " << std::endl;
    kmeans(filtered_image, labels, centers, n_clusters);
    std::cout << "Кластеризация завершена " << std::endl;

    // Преобразование меток обратно в двумерный массив для исходного изображения
    std::vector<std::vector<int>> clustered_image(height, std::vector<int>(width, -1)); // Инициализируем как -1

    // Присваиваем метки только отфильтрованным пикселям
    for (size_t i = 0; i < filtered_indices.size(); ++i) {
        int original_index = filtered_indices[i];
        int row = original_index / width;
        int col = original_index % width;
        clustered_image[row][col] = labels[i];
    }

    // Обработка каждого кластера
    for (int i = 0; i < n_clusters; ++i) {
        std::cout << "Нахождение областей для кластера " << i + 1 << " с цветом ["
            << (int)centers[i].r << ", " << (int)centers[i].g << ", " << (int)centers[i].b << "]:" << std::endl;

        // Поиск и сортировка компонент
        std::vector<Component> components = find_connected_components(clustered_image, i, min_component_size, width, height);
        std::vector<Component> sorted_components = sort_components(components);

        // Выводим и сохраняем изображения компонент
        for (size_t j = 0; j < sorted_components.size(); ++j) {
            std::vector<RGB> highlighted = highlight_components(image, sorted_components[j], width, height);
            std::string filename = "cluster_" + std::to_string(i + 1) + "_component_" + std::to_string(j + 1) + ".jpg";
            save_image_jpg(filename, highlighted, width, height);
            std::cout << "Сохранено изображение: " << filename << std::endl;
        }
    }
}


int main() {
    setlocale(LC_ALL, "Russian");
    std::cout << "Начинаем выполнение программы " << std::endl;
    main_process("example2.jpg", 7, 500);
    return 0;
}
