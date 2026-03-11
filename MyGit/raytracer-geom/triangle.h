#pragma once

#include <cstddef>

#include "vector.h"

class Triangle {
public:
    Triangle() {
    }
    Triangle(const Vector& a, const Vector& b, const Vector& c) : a_(a), b_(b), c_(c) {
    }

    const Vector& operator[](size_t ind) const {
        if (ind == 0) {
            return a_;
        }
        if (ind == 1) {
            return b_;
        }
        return c_;
    }
    double Area() const {
        Vector ab, ac;
        for (int i = 0; i < 3; i++) {
            ab[i] = b_[i] - a_[i];
            ac[i] = c_[i] - a_[i];
        }
        return Length(CrossProduct(ab, ac)) / 2;
    }

private:
    Vector a_, b_, c_;
};
