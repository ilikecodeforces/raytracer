#pragma once

#include <filesystem>
#include <array>
#include <iostream>

#include "image.h"
#include "scene.h"
#include "geometry.h"
#include "camera_options.h"
#include "render_options.h"

struct BasisCamera {
    Vector forward;
    Vector right;
    Vector up;
    BasisCamera(const CameraOptions& camera_opt) {
        forward = (camera_opt.look_to - camera_opt.look_from).Normalize();
        right = CrossProduct(forward, Vector(0.0, 1.0, 0.0));
        if (Length(right) < 1e-6) {
            if (DotProduct(forward, Vector(0.0, 1.0, 0.0)) > 0) {
                right = CrossProduct(forward, Vector(0.0, 0.0, 1.0));
            } else {
                right = CrossProduct(forward, Vector(0.0, 0.0, -1.0));
            }
        }
        right.Normalize();
        up = CrossProduct(right, forward);
        up.Normalize();
    }
};

Ray GetRay(int pixelx, int pixely, const BasisCamera& bc, const CameraOptions& co) {
    double pixel_nd_cx = (pixelx + 0.5) / static_cast<double>(co.screen_width);
    double pixel_nd_cy = (pixely + 0.5) / static_cast<double>(co.screen_height);

    double ratio = co.screen_width / static_cast<double>(co.screen_height);
    double pixel_camerax = (2.0 * pixel_nd_cx - 1.0) * ratio * tan(co.fov / 2.0);
    double pixel_cameray = (1.0 - 2.0 * pixel_nd_cy) * tan(co.fov / 2.0);

    Vector screen_point =
        co.look_from + bc.forward + bc.right * pixel_camerax + bc.up * pixel_cameray;
    Vector direction = screen_point - co.look_from;
    direction.Normalize();

    return Ray(co.look_from, direction);
}

std::array<int, 3> ToneMapping(Vector& v_in, double c) {
    if (c < 1e-8) {
        return {0, 0, 0};
    }

    std::array<double, 3> a;
    for (int i = 0; i < 3; i++) {
        a[i] = (v_in[i] * (1 + (v_in[i] / (c * c)))) / (1 + v_in[i]);
    }
    Vector v_out = Vector(a[0], a[1], a[2]);
    Vector v_gamma =
        Vector(std::pow(a[0], 0.454545), std::pow(a[1], 0.454545), std::pow(a[2], 0.454545));
    int r = v_gamma[0] * 255;
    int g = v_gamma[1] * 255;
    int b = v_gamma[2] * 255;
    return {r, g, b};
}

std::pair<std::optional<Intersection>, const Material*> GetIntersection(Ray ray, Scene& scene) {
    double min_d = std::numeric_limits<double>::max();
    std::optional<Intersection> min_intersection;
    const Material* mat = nullptr;
    for (auto& sphere : scene.GetSphereObjects()) {
        auto intersect = GetIntersection(ray, sphere.sphere);
        if (intersect.has_value()) {
            if (intersect->GetDistance() < min_d) {
                min_d = intersect->GetDistance();
                min_intersection = intersect;
                mat = sphere.material;
            }
        }
    }

    Object polygon;
    bool is_triangle = false;
    for (auto& triangle : scene.GetObjects()) {
        auto intersect = GetIntersection(ray, triangle.polygon);
        if (intersect.has_value()) {
            if (intersect->GetDistance() < min_d) {
                polygon = triangle;
                min_d = intersect->GetDistance();
                min_intersection = intersect;
                mat = triangle.material;
                is_triangle = true;
            }
        }
    }

    if (polygon.has_normal_vertex && is_triangle) {
        Vector pt = GetBarycentricCoords(polygon.polygon, min_intersection->GetPosition());
        Vector n = pt[0] * polygon.vn1 + pt[1] * polygon.vn2 + pt[2] * polygon.vn3;
        n.Normalize();
        if (DotProduct(min_intersection->GetNormal(), n) < 0) {
            n = -1.0 * n;
        }
        min_intersection =
            Intersection(min_intersection->GetPosition(), n, min_intersection->GetDistance());
    }

    return {min_intersection, mat};
}

Vector Illumination(Ray ray, Scene& scene, std::optional<Intersection> min_intersection,
                    const Material* mat) {
    Vector sum_lights;
    if (min_intersection.has_value()) {
        Vector d_i, s_i;
        for (auto& lights : scene.GetLights()) {
            Vector vl = lights.position - min_intersection->GetPosition();
            Vector n = min_intersection->GetNormal();
            double dist = Length(vl);
            vl.Normalize();
            bool in_shadow = false;

            Ray shadow_ray(min_intersection->GetPosition() + 1e-5 * n, vl);
            for (auto& sphere : scene.GetSphereObjects()) {
                auto intersect = GetIntersection(shadow_ray, sphere.sphere);
                if (intersect.has_value()) {
                    double dist_to_obj = intersect->GetDistance();
                    if (dist_to_obj > 1e-5 && dist_to_obj < dist - 1e-5) {
                        in_shadow = true;
                        break;
                    }
                }
            }

            for (auto& triangle : scene.GetObjects()) {
                auto intersect = GetIntersection(shadow_ray, triangle.polygon);
                if (intersect.has_value()) {
                    double dist_to_obj = intersect->GetDistance();
                    if (dist_to_obj > 1e-5 && dist_to_obj < dist - 1e-5) {
                        in_shadow = true;
                        break;
                    }
                }
            }

            if (in_shadow) {
                continue;
            }

            d_i = d_i + lights.intensity * std::max(0.0, DotProduct(n, vl));

            Vector v = -1.0 * ray.GetDirection();
            Vector r = Reflect(-1.0 * vl, n);
            s_i = s_i + lights.intensity *
                            std::pow(std::max(0.0, DotProduct(v, r)), mat->specular_exponent);
        }
        sum_lights = Vector(mat->diffuse_color[0] * d_i[0], mat->diffuse_color[1] * d_i[1],
                            mat->diffuse_color[2] * d_i[2]) +
                     Vector(mat->specular_color[0] * s_i[0], mat->specular_color[1] * s_i[1],
                            mat->specular_color[2] * s_i[2]);
        sum_lights = mat->albedo[0] * sum_lights;
        sum_lights = sum_lights + mat->ambient_color + mat->intensity;
    }
    return sum_lights;
}

Vector TraceRay(Ray ray, Scene& scene, int depth, bool outside) {
    if (depth <= 0) {
        return Vector(0.0, 0.0, 0.0);
    }
    auto intersect = GetIntersection(ray, scene);
    std::optional<Intersection> i = intersect.first;
    const Material* mat = intersect.second;

    if (!i.has_value()) {
        return Vector();
    }

    Vector ans;
    ans = Illumination(ray, scene, i, mat);
    if (mat->albedo[1] > 0 && outside) {
        Vector r = Reflect(ray.GetDirection(), i->GetNormal());
        Ray r_ray = Ray(i->GetPosition() + i->GetNormal() * 1e-5, r);
        Vector ref = TraceRay(r_ray, scene, depth - 1, outside);
        ans = ans + ref * mat->albedo[1];
    }

    if (mat->albedo[2] > 0) {
        double eta = mat->refraction_index;
        if (outside) {
            eta = 1.0 / mat->refraction_index;
        }
        auto r = Refract(ray.GetDirection(), i->GetNormal(), eta);
        if (r.has_value()) {
            Ray r_ray = Ray(i->GetPosition() - 1e-5 * i->GetNormal(), r.value());
            Vector refract = TraceRay(r_ray, scene, depth - 1, !outside);
            ans = ans + refract * (outside ? mat->albedo[2] : 1);
        }
    }
    return ans;
}

Image Render(const std::filesystem::path& path, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    Scene scene = ReadScene(path);
    int w = camera_options.screen_width, h = camera_options.screen_height;
    std::vector<std::vector<Vector>> colors(h, std::vector<Vector>(w, Vector()));

    Image image(w, h);
    BasisCamera basis_camera = BasisCamera(camera_options);
    double c = 0.0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            Ray ray = GetRay(j, i, basis_camera, camera_options);
            colors[i][j] = TraceRay(ray, scene, render_options.depth, 1);
            c = std::max({c, colors[i][j][0], colors[i][j][1], colors[i][j][2]});
        }
    }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            std::array<int, 3> rgb = ToneMapping(colors[i][j], c);
            image.SetPixel({rgb[0], rgb[1], rgb[2]}, i, j);
        }
    }

    return image;
}
