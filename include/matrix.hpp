// include/matrix.hpp

#pragma once

#include <array>
#include <cmath>
#include <cstddef>

#include "vector3.hpp"

template <std::size_t Rows, std::size_t Cols>
class Matrix {
public:
    Matrix() {
        data.fill(0.0f);
    }

    float& operator()(std::size_t row, std::size_t col) {
        return data[row * Cols + col];
    }
 
    float operator()(std::size_t row, std::size_t col) const {
        return data[row * Cols + col];
    }

    static constexpr std::size_t rows() { return Rows; }
    static constexpr std::size_t cols() { return Cols; }

    static Matrix identity() {
        static_assert(Rows == Cols, "identity() requires a square matrix");
        Matrix result;
        for (std::size_t i = 0; i < Rows; i++)
            result(i, i) = 1.0f;

        return result;
    }

    // Matrix multiplication: (Rows x Cols) * (Cols x OtherCols) -> (Rows x OtherCols)
    template <std::size_t OtherCols>
    Matrix<Rows, OtherCols> operator*(const Matrix<Cols, OtherCols>& other) const {
        Matrix<Rows, OtherCols> result;
        for (std::size_t i = 0; i < Rows; i++) {
            for (std::size_t j = 0; j < OtherCols; j++) {
                float sum = 0.0f;
                for (std::size_t k = 0; k < Cols; k++) {
                    sum += (*this)(i, k) * other(k, j);
                }
                result(i, j) = sum;
            }
        }
        return result;
    }

       Matrix operator*(float scalar) const {
        Matrix result;
        for (std::size_t i = 0; i < Rows * Cols; i++) result.data[i] = data[i] * scalar;
        return result;
    }
 
    Matrix operator+(const Matrix& other) const {
        Matrix result;
        for (std::size_t i = 0; i < Rows * Cols; i++) result.data[i] = data[i] + other.data[i];
        return result;
    }
 
    Matrix operator-(const Matrix& other) const {
        Matrix result;
        for (std::size_t i = 0; i < Rows * Cols; i++) result.data[i] = data[i] - other.data[i];
        return result;
    }
 
    Matrix<Cols, Rows> transpose() const {
        Matrix<Cols, Rows> result;
        for (std::size_t i = 0; i < Rows; i++)
            for (std::size_t j = 0; j < Cols; j++)
                result(j, i) = (*this)(i, j);
        return result;
    }
 
private:
    std::array<float, Rows * Cols> data;
};
 
using Matrix3 = Matrix<3, 3>;
using Matrix4 = Matrix<4, 4>;
 
// Multiply a 3x3 matrix by a Vector3, treating the vector as a column vector.
inline Vector3 operator*(const Matrix3& m, const Vector3& v) {
    return Vector3(
        m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z,
        m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z,
        m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z
    );
}
 
// Rotation matrix builders (angle in radians).
inline Matrix3 rotationX(float angle) {
    float c = std::cos(angle), s = std::sin(angle);
    Matrix3 m = Matrix3::identity();
    m(1, 1) = c;  m(1, 2) = -s;
    m(2, 1) = s;  m(2, 2) = c;
    return m;
}
 
inline Matrix3 rotationY(float angle) {
    float c = std::cos(angle), s = std::sin(angle);
    Matrix3 m = Matrix3::identity();
    m(0, 0) = c;  m(0, 2) = s;
    m(2, 0) = -s; m(2, 2) = c;
    return m;
}
 
inline Matrix3 rotationZ(float angle) {
    float c = std::cos(angle), s = std::sin(angle);
    Matrix3 m = Matrix3::identity();
    m(0, 0) = c; m(0, 1) = -s;
    m(1, 0) = s; m(1, 1) = c;
    return m;
}