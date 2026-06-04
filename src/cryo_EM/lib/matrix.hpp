#ifndef MATRIX_HPP
#define MATRIX_HPP
#include <string.h>
template <typename T>
class Matrix {
private:
    int row, col;

public:
    Matrix(int row, int col) : row(row), col(col) {
        data = new T *[row];
        for (int i = 0; i < row; i++) {
            data[i] = new T[col]();
        }
    }
    ~Matrix() {
        for (int i = 0; i < row; i++) {
            delete[] data[i];
        }
        delete[] data;
    }
    T *operator[](int i) const {
        return data[i];
    }
    int get_row_num() const {
        return row;
    }
    int get_col_num() const {
        return col;
    }
    Matrix<T> operator*(const Matrix<T> &m) const {
        if (col != m.row) {
            throw "Matrix size error";
        }
        Matrix<T> result(row, m.col);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < m.col; j++) {
                result[i][j] = 0;
                for (int k = 0; k < row; k++) {
                    result[i][j] += data[i][k] * m[k][j];
                }
            }
        }
        return result;
    }
    Matrix<T> operator+(const Matrix<T> &m) const {
        if (row != m.row || col != m.col) {
            throw "Matrix size error";
        }
        Matrix<T> result(*this);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                result[i][j] += m[i][j];
            }
        }
        return result;
    }
    Matrix<T> operator-(const Matrix<T> &m) const {
        if (row != m.row || col != m.col) {
            throw "Matrix size error";
        }
        Matrix<T> result(*this);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                result[i][j] -= m[i][j];
            }
        }
        return result;
    }
    Matrix<T> operator+=(const Matrix<T> &m) {
        if (row != m.row || col != m.col) {
            throw "Matrix size error";
        }
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                data[i][j] += m[i][j];
            }
        }
        return *this;
    }
    Matrix<T> operator-=(const Matrix<T> &m) {
        if (row != m.row || col != m.col) {
            throw "Matrix size error";
        }
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                data[i][j] -= m[i][j];
            }
        }
        return *this;
    }
    Matrix<T> operator=(const Matrix<T> &m) {
        if (row != m.row || col != m.col) {
            throw "Matrix size error";
        }
        for (int i = 0; i < row; i++) {
            memcpy(data[i], m[i], sizeof(T) * col);
        }
        return *this;
    }
    Matrix<T> operator*=(const T &k) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                data[i][j] *= k;
            }
        }
        return *this;
    }
    Matrix<T> operator/=(const T &k) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                data[i][j] /= k;
            }
        }
        return *this;
    }
    Matrix<T> operator+=(const T &k) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                data[i][j] += k;
            }
        }
        return *this;
    }
    Matrix<T> operator-=(const T &k) {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                data[i][j] -= k;
            }
        }
        return *this;
    }
    Matrix<T> operator*(const T &k) const {
        Matrix<T> result(*this);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                result[i][j] *= k;
            }
        }
        return result;
    }
    Matrix<T> operator/(const T &k) const {
        Matrix<T> result(*this);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                result[i][j] /= k;
            }
        }
        return result;
    }
    Matrix<T> operator+(const T &k) const {
        Matrix<T> result(*this);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                result[i][j] += k;
            }
        }
        return result;
    }
    Matrix<T> operator-(const T &k) const {
        Matrix<T> result(*this);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                result[i][j] -= k;
            }
        }
        return result;
    }
    Matrix(const Matrix<T> &m) {  // 拷贝构造函数
        row = m.row, col = m.col;
        data = new T *[row];
        for (int i = 0; i < row; i++) {
            data[i] = new T[col]();
            memcpy(data[i], m[i], sizeof(T) * col);
        }
    }
    T **data;
};

#endif  // MATRIX_HPP