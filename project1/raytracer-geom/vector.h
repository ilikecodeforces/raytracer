#pragma once

#include <array>
#include <cmath>
#include <cstddef>

class Vector {
public:
    Vector() {
        data_.fill(0.0);
    }
    Vector(double x, double y, double z) {
        data_[0] = x;
        data_[1] = y;
        data_[2] = z;
    };
    Vector(const Vector& other) {
        data_[0] = other[0];
        data_[1] = other[1];
        data_[2] = other[2];
    }

    double& operator[](size_t ind) {
        return data_[ind];
    }
    double operator[](size_t ind) const {
        return data_[ind];
    }
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            data_[1] = other.data_[1];
            data_[2] = other.data_[2];
            data_[0] = other.data_[0];
        }
        return *this;
    }
    Vector operator+(const Vector& other) const {
        return Vector(data_[0] + other[0], data_[1] + other[1], data_[2] + other[2]);
    }
    Vector operator-(const Vector& other) const {
        return Vector(data_[0] - other[0], data_[1] - other[1], data_[2] - other[2]);
    }
    Vector operator*(double scalar) const {
        return Vector(data_[0] * scalar, data_[1] * scalar, data_[2] * scalar);
    }
    friend Vector operator*(double scalar, const Vector& vec) {
        return vec * scalar;
    }

    Vector& Normalize() {
        double n = data_[0] * data_[0] + data_[1] * data_[1] + data_[2] * data_[2];
        if (n == 1) {
            return *this;
        }
        n = std::sqrt(n);
        if (n > 1e-9) {
            data_[0] = data_[0] / n;
            data_[1] = data_[1] / n;
            data_[2] = data_[2] / n;
        } else {
            data_[0] = 0.0;
            data_[1] = 0.0;
            data_[2] = 0.0;
        }
        return *this;
    }

private:
    std::array<double, 3> data_;
};

double DotProduct(const Vector& a, const Vector& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
Vector CrossProduct(const Vector& a, const Vector& b) {
    Vector cross;
    cross[0] = a[1] * b[2] - a[2] * b[1];
    cross[1] = a[2] * b[0] - a[0] * b[2];
    cross[2] = a[0] * b[1] - a[1] * b[0];
    return cross;
}
double Length(const Vector& v) {
    return std::sqrt(DotProduct(v, v));
}
