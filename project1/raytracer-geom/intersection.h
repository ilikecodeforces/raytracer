#pragma once

#include "vector.h"

class Intersection {
public:
    Intersection(const Vector& position, const Vector& normal, double distance)
        : intersect_(position), normal_(normal), dist_(distance) {};

    Intersection(const Intersection& other)
        : intersect_(other.intersect_), normal_(other.normal_), dist_(other.dist_) {
    }
    Intersection& operator=(const Intersection& other) {
        if (this != &other) {
            for (int i = 0; i < 3; i++) {
                intersect_[i] = other.intersect_[i];
                normal_[i] = other.normal_[i];
            }
            dist_ = other.dist_;
        }
        return *this;
    }

    const Vector& GetPosition() const {
        return intersect_;
    }
    const Vector& GetNormal() const {
        return normal_;
    }
    double GetDistance() const {
        return dist_;
    }

private:
    Vector intersect_, normal_;
    double dist_;
};
