//
// Created by 潘鑫 on 2026/5/24.
//

#include "parse_geometry_file.h"

#include "tiny_obj_loader.h"


bool load_obj_file(const std::string &path,
                   const std::shared_ptr<std::vector<Vertex> > &vertices,
                   const std::shared_ptr<std::vector<uint16_t> > &indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    auto result = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, path.c_str());
    // todo : result need check
    if (result == false) {
        return false;
    }
    // Load vertex and index data
    for (const auto &index: shapes[0].mesh.indices) {
        Vertex v{
            .pos = {
                attrib.vertices[index.vertex_index * 3 + 0],
                -attrib.vertices[index.vertex_index * 3 + 1],
                attrib.vertices[index.vertex_index * 3 + 2]
            },
            .normal = {
                attrib.normals[index.normal_index * 3 + 0],
                -attrib.normals[index.normal_index * 3 + 1],
                attrib.normals[index.normal_index * 3 + 2]
            },
            .uv = {
                attrib.texcoords[index.texcoord_index * 2],
                1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]
            }
        };
        vertices->push_back(v);
        indices->push_back(indices->size());
    }
}


std::optional<AABB_min_max<Point_3> > load_model(const entt::entity entity, const std::string &path) {
    auto sp_vertices                     = std::make_shared<std::vector<Vertex> >();
    auto sp_indices                      = std::make_shared<std::vector<uint16_t> >();
    const std::filesystem::path filePath = path;
    const std::string ext                = filePath.extension().string();

    if (ext == ".obj") {
        load_obj_file(path, sp_vertices, sp_indices);
        add_geometry_data(entity, sp_vertices, sp_indices);
        return {};
    } else {
        return {};
    }
    return {};
}
