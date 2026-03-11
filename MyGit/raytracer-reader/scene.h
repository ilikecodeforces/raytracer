#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "light.h"
#include "material.h"
#include "object.h"
#include "vector.h"

class Scene {
private:
    std::vector<Object> objects_;
    std::vector<SphereObject> sphere_objects_;
    std::vector<Light> lights_;
    std::unordered_map<std::string, Material> materials_;

public:
    Scene(std::vector<Object>&& objects, std::vector<SphereObject>&& sphere_objects,
          std::vector<Light>&& lights,
          std::unordered_map<std::string, Material>&& materials) noexcept
        : objects_(std::move(objects)),
          sphere_objects_(std::move(sphere_objects)),
          lights_(std::move(lights)),
          materials_(std::move(materials)) {
    }

    const std::vector<Object>& GetObjects() const {
        return objects_;
    }
    const std::vector<SphereObject>& GetSphereObjects() const {
        return sphere_objects_;
    }
    const std::vector<Light>& GetLights() const {
        return lights_;
    }
    const std::unordered_map<std::string, Material>& GetMaterials() const {
        return materials_;
    }
};

std::unordered_map<std::string, Material> ReadMaterials(const std::filesystem::path& path) {
    std::unordered_map<std::string, Material> materials;
    std::ifstream file(path);

    std::string line;
    bool newmtl = false;
    Material material;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "newmtl") {
            if (newmtl) {
                materials[material.name] = material;
            }

            std::string name;
            iss >> name;

            material = Material();
            material.name = name;
            newmtl = true;
        } else if (newmtl) {
            if (prefix == "al") {
                double r, g, b;
                if (iss >> r >> g >> b) {
                    material.albedo = Vector(r, g, b);
                }
            } else if (prefix == "Ns") {
                double ns;
                if (iss >> ns) {
                    material.specular_exponent = ns;
                }
            } else if (prefix == "Ni") {
                double ni;
                if (iss >> ni) {
                    material.refraction_index = ni;
                }
            } else if (prefix == "Ke") {
                double r, g, b;
                if (iss >> r >> g >> b) {
                    material.intensity = Vector(r, g, b);
                }
            } else if (prefix == "Ks") {
                double r, g, b;
                if (iss >> r >> g >> b) {
                    material.specular_color = Vector(r, g, b);
                }
            } else if (prefix == "Kd") {
                double r, g, b;
                if (iss >> r >> g >> b) {
                    material.diffuse_color = Vector(r, g, b);
                }
            } else if (prefix == "Ka") {
                double r, g, b;
                if (iss >> r >> g >> b) {
                    material.ambient_color = Vector(r, g, b);
                }
            }
        }

        if (newmtl) {
            materials[material.name] = material;
        }
    }
    return materials;
}

bool ParseFaceVertex(const std::string& token, int& v_idx, int& vt_idx, int& vn_idx, size_t n,
                     size_t m) {
    v_idx = vt_idx = vn_idx = 0;

    size_t pos1 = token.find('/');
    if (pos1 == std::string::npos) {
        v_idx = std::stoi(token);
        if (v_idx < 0) {
            v_idx += n;
        } else {
            v_idx--;
        }
        return false;
    }

    v_idx = std::stoi(token.substr(0, pos1));
    if (v_idx < 0) {
        v_idx += n;
    } else {
        v_idx--;
    }

    size_t pos2 = token.find('/', pos1 + 1);
    if (pos2 == std::string::npos) {
        vt_idx = std::stoi(token.substr(pos1 + 1));
        return false;
    }

    if (pos2 == pos1 + 1) {
        vn_idx = std::stoi(token.substr(pos2 + 1));
        if (vn_idx < 0) {
            vn_idx += m;
        } else {
            vn_idx--;
        }
        return true;
    }

    vt_idx = std::stoi(token.substr(pos1 + 1, pos2 - pos1 - 1));
    vn_idx = std::stoi(token.substr(pos2 + 1));
    if (vn_idx < 0) {
        vn_idx += m;
    } else {
        vn_idx--;
    }
    return true;
}

Scene ReadScene(const std::filesystem::path& path) {
    std::ifstream file(path);

    std::vector<Object> objects;
    std::vector<Vector> vertex;
    std::vector<Vector> normals;
    std::vector<SphereObject> sphere_object;
    std::vector<Light> lights;
    std::unordered_map<std::string, Material> materials;
    std::string current_material;

    std::string line;
    bool usemtl = false;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "mtllib") {
            std::string mfile;
            iss >> mfile;
            std::filesystem::path materials_path = path.parent_path() / mfile;
            materials = ReadMaterials(materials_path);
        } else {
            if (prefix == "v") {
                double x, y, z;
                iss >> x >> y >> z;
                vertex.push_back({x, y, z});
            } else if (prefix == "vn") {
                double nx, ny, nz;
                iss >> nx >> ny >> nz;
                normals.push_back({nx, ny, nz});
            } else if (prefix == "S") {
                double r, x, y, z;
                iss >> x >> y >> z >> r;
                if (usemtl) {
                    sphere_object.push_back(
                        SphereObject(Sphere(x, y, z, r), &materials[current_material]));
                } else {
                    sphere_object.push_back(SphereObject(Sphere(x, y, z, r)));
                }
            } else if (prefix == "P") {
                double x, y, z, r, g, b;
                iss >> x >> y >> z >> r >> g >> b;
                lights.push_back({Vector(x, y, z), Vector(r, g, b)});
            } else if (prefix == "usemtl") {
                usemtl = true;
                iss >> current_material;
            } else if (prefix == "f") {
                std::vector<std::string> tokens;
                std::string token;
                while (iss >> token) {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 3) {
                    int v0, vt0, vn0;
                    bool has_vn0;
                    has_vn0 =
                        ParseFaceVertex(tokens[0], v0, vt0, vn0, vertex.size(), normals.size());

                    for (size_t i = 1; i + 1 < tokens.size(); i++) {
                        int v1, vt1, vn1;
                        int v2, vt2, vn2;
                        bool has_vn2, has_vn1;

                        has_vn1 =
                            ParseFaceVertex(tokens[i], v1, vt1, vn1, vertex.size(), normals.size());
                        has_vn2 = ParseFaceVertex(tokens[i + 1], v2, vt2, vn2, vertex.size(),
                                                  normals.size());

                        Triangle t(vertex[v0], vertex[v1], vertex[v2]);
                        Vector n0, n1, n2;
                        if (has_vn0) {
                            n0 = normals[vn0];
                        }
                        if (has_vn1) {
                            n1 = normals[vn1];
                        }
                        if (has_vn2) {
                            n2 = normals[vn2];
                        }

                        if (usemtl) {
                            if (has_vn0 && has_vn1 && has_vn2) {
                                objects.push_back(
                                    Object(t, n0, n1, n2, &materials[current_material]));
                            } else {
                                objects.push_back(Object(t, &materials[current_material]));
                            }
                        } else {
                            if (has_vn0 && has_vn1 && has_vn2) {
                                objects.push_back(Object(t, n0, n1, n2));
                            } else {
                                objects.push_back(Object(t));
                            }
                        }
                    }
                }
            }
        }
    }

    return Scene(std::move(objects), std::move(sphere_object), std::move(lights),
                 std::move(materials));
}
