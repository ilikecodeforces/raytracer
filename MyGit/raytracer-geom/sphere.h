#pragma once

#include "vector.h"

class Sphere {
public:
    Sphere(double x, double y, double z, double r) : center_(Vector(x, y, z)), radius_(r) {
    }
    Sphere(const Vector& center, double radius) : center_(center), radius_(radius) {};

    const Vector& GetCenter() const {
        return center_;
    }
    double GetRadius() const {
        return radius_;
    }

private:
    Vector center_;
    double radius_;
};
