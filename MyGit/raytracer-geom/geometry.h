#pragma once

#include <cmath>
#include <optional>

#include "intersection.h"
#include "ray.h"
#include "sphere.h"
#include "triangle.h"
#include "vector.h"

const double kEps = 1e-9;

double Dot(const Vector& a, const Vector& b, const Vector& c) {
    return a[0] * (b[1] * c[2] - c[1] * b[2]) - b[0] * (a[1] * c[2] - c[1] * a[2]) +
           c[0] * (a[1] * b[2] - b[1] * a[2]);
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    Vector o = ray.GetOrigin(), d = ray.GetDirection(), center = sphere.GetCenter();
    double a = DotProduct(d, d);
    double r = sphere.GetRadius();

    Vector co;
    for (int i = 0; i < 3; i++) {
        co[i] = o[i] - center[i];
    }
    double b = 2 * DotProduct(d, co);
    double c = DotProduct(co, co) - r * r;

    double dt = b * b - 4 * a * c;
    if (dt < 0) {
        return std::nullopt;
    }

    double t1, t2;
    t1 = (-b + std::sqrt(dt)) / (2.0 * a);
    t2 = (-b - std::sqrt(dt)) / (2.0 * a);

    double t = -1.0;
    if (t1 >= 0 && t2 >= 0) {
        t = std::min(t1, t2);
    } else if (t1 >= 0) {
        t = t1;
    } else if (t2 >= 0) {
        t = t2;
    } else {
        return std::nullopt;
    }

    Vector pos, n;
    double dist = 0.0;
    for (int i = 0; i < 3; i++) {
        pos[i] = o[i] + t * d[i];
        dist += (pos[i] - o[i]) * (pos[i] - o[i]);
        n[i] = pos[i] - center[i];
    }

    dist = std::sqrt(dist);
    if (DotProduct(n, d) > 0) {
        for (int i = 0; i < 3; i++) {
            n[i] *= -1;
        }
    }
    n.Normalize();
    Intersection intersect = {pos, n, dist};
    return intersect;
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    Vector ab, ac, d = ray.GetDirection(), o = ray.GetOrigin(), neg_d;
    for (int i = 0; i < 3; i++) {
        ab[i] = triangle[1][i] - triangle[0][i];
        ac[i] = triangle[2][i] - triangle[0][i];
        neg_d[i] = -d[i];
    }

    Vector n = CrossProduct(ab, ac);
    if (std::fabs(DotProduct(n, ray.GetDirection())) <= kEps) {
        return std::nullopt;
    }

    double det = Dot(neg_d, ab, ac);
    double invdet = 1.0 / det;
    Vector oa;
    for (int i = 0; i < 3; i++) {
        oa[i] = o[i] - triangle[0][i];
    }

    double t = invdet * Dot(oa, ab, ac);
    double u = invdet * Dot(neg_d, oa, ac);
    double v = invdet * Dot(neg_d, ab, oa);

    if (u < -kEps || u > 1.0 + kEps) {
        return std::nullopt;
    }
    if (v < kEps || v > 1.0 + kEps) {
        return std::nullopt;
    }
    if (u + v > 1.0 + kEps) {
        return std::nullopt;
    }
    if (t <= kEps) {
        return std::nullopt;
    }
    Vector pos;
    for (int i = 0; i < 3; i++) {
        pos[i] = o[i] + t * d[i];
    }

    Vector normal = n;
    if (DotProduct(d, normal) > kEps) {
        for (int i = 0; i < 3; i++) {
            normal[i] *= -1;
        }
    }
    normal.Normalize();

    double dist = 0.0;
    for (int i = 0; i < 3; i++) {
        dist += (o[i] - pos[i]) * (o[i] - pos[i]);
    }
    dist = std::sqrt(dist);
    Intersection intersection = {pos, normal, dist};
    return intersection;
}

Vector Reflect(const Vector& ray, const Vector& normal) {
    Vector reflect;
    double d = DotProduct(normal, ray);
    for (int i = 0; i < 3; i++) {
        reflect[i] = ray[i] - 2 * d * normal[i];
    }
    reflect.Normalize();
    return reflect;
}
std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    double k = 1.0 - eta * eta * (1.0 - DotProduct(normal, ray) * DotProduct(normal, ray));
    if (k < kEps) {
        return std::nullopt;
    } else {
        Vector refract;
        for (int i = 0; i < 3; i++) {
            refract[i] = eta * ray[i] - (eta * DotProduct(normal, ray) + std::sqrt(k)) * normal[i];
        }
        refract.Normalize();
        return refract;
    }
};
Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    Vector ab, ac, pb, pc, pa;
    for (int i = 0; i < 3; i++) {
        ab[i] = triangle[1][i] - triangle[0][i];
        ac[i] = triangle[2][i] - triangle[0][i];
        pa[i] = triangle[0][i] - point[i];
        pb[i] = triangle[1][i] - point[i];
        pc[i] = triangle[2][i] - point[i];
    }

    double area = Length(CrossProduct(ab, ac));
    double u = Length(CrossProduct(pb, pc)) / area;
    double v = Length(CrossProduct(pa, pc)) / area;
    double w = 1.0 - u - v;
    Vector coordinate = {u, v, w};
    return coordinate;
}
