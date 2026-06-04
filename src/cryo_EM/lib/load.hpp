#ifndef LOAD_HPP
#define LOAD_HPP
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdexcept>

#include <QImage>
#include <string>

#include "matrix.hpp"
using std::string;
class Images {
private:
    uint32_t shape[3] = {0, 0, 0};
    uint8_t*** data = nullptr;
    uint8_t* array = nullptr;

    void clear() {
        if (data == nullptr) return;
        for (uint32_t i = 0; i < shape[0]; ++i)
            delete[] data[i];
        delete[] data;
        delete[] array;
        data = nullptr, array = nullptr, shape[0] = shape[1] = shape[2] = 0;
    }

public:
    Images() {}
    Images(const string& file_path) {
        load(file_path);
    }
    ~Images() {
        clear();
    }
    bool load(const string& file_path) {
        if (data != nullptr) clear();

        FILE* file = fopen(file_path.c_str(), "rb");
        if (file == nullptr) {
            fprintf(stderr, "Error: Cannot open file: %s\n", file_path.c_str());
            return false;
        }

        size_t read_count = fread(&(shape[0]), sizeof(uint32_t), 3, file);
        if (read_count != 3) {
            fprintf(stderr, "Error: Failed to read shape from file\n");
            fclose(file);
            return false;
        }

        if (shape[0] == 0 || shape[1] == 0 || shape[2] == 0) {
            fprintf(stderr, "Error: Invalid shape: [%u, %u, %u]\n", shape[0], shape[1], shape[2]);
            fclose(file);
            return false;
        }

        size_t total_size = (size_t)shape[0] * shape[1] * shape[2];
        const size_t MAX_SIZE = 4ULL * 1024 * 1024 * 1024;
        if (total_size > MAX_SIZE) {
            fprintf(stderr, "Error: Data too large: %zu bytes (max %zu)\n", total_size, MAX_SIZE);
            fclose(file);
            return false;
        }

        printf("Loading data with shape [%u, %u, %u], total %zu bytes (%.2f MB)\n",
               shape[0], shape[1], shape[2], total_size, total_size / (1024.0 * 1024.0));
        fflush(stdout);

        try {
            array = new uint8_t[total_size];
        } catch (const std::bad_alloc& e) {
            fprintf(stderr, "Error: Memory allocation failed: %s\n", e.what());
            fclose(file);
            shape[0] = shape[1] = shape[2] = 0;
            return false;
        }

        read_count = fread(array, sizeof(uint8_t), total_size, file);
        if (read_count != total_size) {
            fprintf(stderr, "Error: Failed to read data (expected %zu, got %zu)\n", total_size, read_count);
            delete[] array;
            array = nullptr;
            fclose(file);
            shape[0] = shape[1] = shape[2] = 0;
            return false;
        }

        try {
            data = new uint8_t**[shape[0]];
            for (uint32_t i = 0; i < shape[0]; ++i) {
                data[i] = new uint8_t*[shape[1]];
                for (uint32_t j = 0; j < shape[1]; ++j)
                    data[i][j] = array + (i * shape[1] + j) * shape[2];
            }
        } catch (const std::bad_alloc& e) {
            fprintf(stderr, "Error: Pointer array allocation failed: %s\n", e.what());
            delete[] array;
            array = nullptr;
            if (data != nullptr) {
                for (uint32_t i = 0; i < shape[0]; ++i) {
                    if (data[i] != nullptr)
                        delete[] data[i];
                }
                delete[] data;
                data = nullptr;
            }
            fclose(file);
            shape[0] = shape[1] = shape[2] = 0;
            return false;
        }

        fclose(file);
        printf("Data loaded successfully\n");
        fflush(stdout);
        return true;
    }
    uint8_t*** get_data_array() {
        return data;
    }
    uint8_t* get_data_array_1d() {
        return array;
    }
    uint32_t* get_shape() {
        return shape;
    }
    QImage clip_slow(int x, int y, int z, int rotate_x_deg, int rotate_y_deg, int rotate_z_deg) {
        if (data == nullptr)
            return QImage();
        assert(0 <= rotate_x_deg && rotate_x_deg <= 180);
        assert(0 <= rotate_y_deg && rotate_y_deg <= 180);
        assert(0 <= rotate_z_deg && rotate_z_deg <= 180);
        double rotate_x = M_PI / 180.0 * rotate_x_deg, rotate_y = M_PI / 180.0 * rotate_y_deg, rotate_z = M_PI / 180.0 * rotate_z_deg;
        Matrix<double> normal(3, 1);
        normal[0][0] = 0;
        normal[1][0] = 0;
        normal[2][0] = 1;
        Matrix<double> rotate_x_matrix(3, 3);
        rotate_x_matrix[0][0] = 1, rotate_x_matrix[0][1] = 0, rotate_x_matrix[0][2] = 0;
        rotate_x_matrix[1][0] = 0, rotate_x_matrix[1][1] = cos(rotate_x), rotate_x_matrix[1][2] = -sin(rotate_x);
        rotate_x_matrix[2][0] = 0, rotate_x_matrix[2][1] = sin(rotate_x), rotate_x_matrix[2][2] = cos(rotate_x);
        Matrix<double> rotate_y_matrix(3, 3);
        rotate_y_matrix[0][0] = cos(rotate_y), rotate_y_matrix[0][1] = 0, rotate_y_matrix[0][2] = sin(rotate_y);
        rotate_y_matrix[1][0] = 0, rotate_y_matrix[1][1] = 1, rotate_y_matrix[1][2] = 0;
        rotate_y_matrix[2][0] = -sin(rotate_y), rotate_y_matrix[2][1] = 0, rotate_y_matrix[2][2] = cos(rotate_y);
        Matrix<double> rotate_z_matrix(3, 3);
        rotate_z_matrix[0][0] = cos(rotate_z), rotate_z_matrix[0][1] = -sin(rotate_z), rotate_z_matrix[0][2] = 0;
        rotate_z_matrix[1][0] = sin(rotate_z), rotate_z_matrix[1][1] = cos(rotate_z), rotate_z_matrix[1][2] = 0;
        rotate_z_matrix[2][0] = 0, rotate_z_matrix[2][1] = 0, rotate_z_matrix[2][2] = 1;
        Matrix<double> rotate_matrix = rotate_z_matrix * rotate_y_matrix * rotate_x_matrix;
        Matrix<double> normal_rotate = rotate_matrix * normal;
        double a = normal_rotate[0][0], b = normal_rotate[1][0], c = normal_rotate[2][0], d = a * x + b * y + c * z, abs_a = abs(a), abs_b = abs(b), abs_c = abs(c);
        if (abs_a >= abs_b && abs_a >= abs_c) {            // 平面在垂直x轴的面上投影面积最大，因此我们将平面投影到yz平面上
            uint32_t width = shape[0], height = shape[1];  // width=z,height=y
            QImage image(width, height, QImage::Format_Grayscale8);
            for (uint32_t i = 0; i < height; ++i) {
                uint8_t* line = image.scanLine(i);
                for (uint32_t j = 0; j < width; ++j) {
                    double tx_calc = (d - b * i - c * j) / a + 0.5;  //+0.5四舍五入
                    if (tx_calc < 0 || tx_calc >= shape[2]) {
                        line[j] = 0;
                    } else {
                        uint32_t tx = (uint32_t)tx_calc;
                        line[j] = data[j][i][tx];  // data[z][y][x]
                    }
                }
            }
            return image;
        }
        if (abs_b >= abs_a && abs_b >= abs_c) {            // 平面在垂直y轴的面上投影面积最大，因此我们将平面投影到xz平面上
            uint32_t width = shape[2], height = shape[0];  // width=x,height=z
            QImage image(width, height, QImage::Format_Grayscale8);
            for (uint32_t i = 0; i < height; ++i) {
                uint8_t* line = image.scanLine(i);
                for (uint32_t j = 0; j < width; ++j) {
                    double ty_calc = (d - a * i - c * j) / b + 0.5;  //+0.5四舍五入
                    if (ty_calc < 0 || ty_calc >= shape[1]) {
                        line[j] = 0;
                    } else {
                        uint32_t ty = (uint32_t)ty_calc;
                        line[j] = data[i][ty][j];  // data[z][y][x]
                    }
                }
            }
            return image;
        }
        // 平面在垂直z轴的面上投影面积最大，因此我们将平面投影到xy平面上
        uint32_t width = shape[2], height = shape[1];  // width=x,height=y
        QImage image(width, height, QImage::Format_Grayscale8);
        for (uint32_t i = 0; i < height; ++i) {
            uint8_t* line = image.scanLine(i);
            for (uint32_t j = 0; j < width; ++j) {
                double tz_calc = (d - a * i - b * j) / c + 0.5;  //+0.5四舍五入
                if (tz_calc < 0 || tz_calc >= shape[0]) {
                    line[j] = 0;
                } else {
                    uint32_t tz = (uint32_t)tz_calc;
                    line[j] = data[tz][i][j];  // 修正：data[z][y][x]，i是y，j是x
                }
            }
        }
        return image;
    }

    QImage getXYImage(int x, int y, int z) {
        if (data == nullptr)
            return QImage();
        return clip_slow(x, y, z, 0, 0, 0);
    }
    QImage getXZImage(int x, int y, int z) {
        if (data == nullptr)
            return QImage();
        return clip_slow(x, y, z, 90, 0, 0);
    }
    QImage getYZImage(int x, int y, int z) {
        if (data == nullptr)
            return QImage();
        return clip_slow(x, y, z, 0, 90, 0);
    }
};

#endif  // LOAD_HPP
