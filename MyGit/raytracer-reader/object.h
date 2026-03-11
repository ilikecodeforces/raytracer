#pragma once

#include "material.h"
#include "sphere.h"
#include "triangle.h"
#include "vector.h"

struct Object {
    Triangle polygon;
    Vector vn1, vn2, vn3;
    const Material* material = nullptr;
    bool has_normal_vertex = false;
    Object() {
    }
    Object(Triangle t, Vector a, Vector b, Vector c, const Material* m = nullptr)
        : polygon(t), vn1(a), vn2(b), vn3(c), material(m) {
        has_normal_vertex = true;
    }
    Object(Triangle t, const Material* m = nullptr) : polygon(t), material(m) {
    }
    const Vector* GetNormal(size_t index) const {
        if (index == 0) {
            return &vn1;
        }
        if (index == 1) {
            return &vn2;
        }
        return &vn3;
    }
};

struct SphereObject {
    Sphere sphere;
    const Material* material = nullptr;

    SphereObject(Sphere s, const Material* m = nullptr) : sphere(s), material(m) {
    }
};
