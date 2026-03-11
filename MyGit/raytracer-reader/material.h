#pragma once

#include <string>

#include "vector.h"

struct Material {
    std::string name;
    Vector ambient_color;
    Vector diffuse_color;
    Vector specular_color;
    Vector intensity;
    double specular_exponent = 1.0;
    double refraction_index = 1.0;
    Vector albedo{1.0, 0.0, 0.0};
};
