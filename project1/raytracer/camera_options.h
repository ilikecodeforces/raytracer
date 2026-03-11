#pragma once

#include <numbers>

#include "vector.h"
#include "ray.h"

struct CameraOptions {
    int screen_width = 640;
    int screen_height = 480;
    double fov = std::numbers::pi / 2;
    Vector look_from = {0., 0., 0.};
    Vector look_to = {0., 0., -1.};
};
