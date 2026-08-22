//
// Created by 潘鑫 on 2026/8/3.
//

#include "manifold_model.h"
#include "manifold/cross_section.h"
#include "manifold/manifold.h"
#include <vector>

#include "base_3d_render_object.h"
#include "earcut.h"
#include "shader_component.h"


using Point       = std::array<double, 2>;
using ear_Polygon = std::vector<std::vector<Point> >;

void triangulateSlice(const manifold::Polygons &manifoldPolys) {
    // 2. 转换 manifold 数据到 earcut 格式
    ear_Polygon polygon;
    for (const auto &ring: manifoldPolys) {
        std::vector<Point> earcut_ring;
        for (const auto &p: ring) {
            earcut_ring.push_back({(double) p.x, (double) p.y});
        }
        polygon.push_back(earcut_ring);
    }

    // 3. 执行三角化
    // 返回的是顶点索引，每 3 个索引代表一个三角形
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

    // 4. 渲染逻辑 (伪代码)
    // for (size_t i = 0; i < indices.size(); i += 3) {
    //     drawTriangle(polygon_flattened[indices[i]], ...);
    // }
}


void add_manifold_entity() { {
        // 创建一个球体模型
        manifold::Manifold sphere = manifold::Manifold::Sphere(10.0f);

        // 在高度 5.0 处切片
        // 返回值是一个 CrossSection 对象，内部封装了 Clipper2 库来处理二维布尔运算
        manifold::CrossSection section = sphere.Slice(5.0f);

        // 导出多边形顶点数据
        manifold::Polygons polys = section.ToPolygons();

        triangulateSlice(polys);
    }
    // 3d 模型
    {
        manifold::Manifold box = manifold::Manifold::Cube({10, 10, 10}, true);
        manifold::MeshGL mesh  = box.GetMeshGL();
    } {
        // 创建两个简单的几何体并取交集
        manifold::Manifold box = manifold::Manifold::Cube({10, 10, 10}, true);

        manifold::MeshGL mesh = box.GetMeshGL();
        mesh.numProp          = 8; // 现在每个顶点占 5 个 float (x, y, z, nx, ny, nz, u, v)
        std::vector<float> newProps;
        newProps.reserve(mesh.NumVert() * mesh.numProp); // 预留空间
        // 4. 为原有的每个顶点补充 UV 数据
        // 注意：mesh.vertProperties 原本只存了 [x0, y0, z0, x1, y1, z1...]
        for (size_t i = 0; i < mesh.vertProperties.size(); i += 3) {
            // 复制 XYZ
            newProps.push_back(mesh.vertProperties[i]);     // x
            newProps.push_back(mesh.vertProperties[i + 1]); // y
            newProps.push_back(mesh.vertProperties[i + 2]); // z

            newProps.push_back(0.0f); // nx
            newProps.push_back(0.0f); // ny
            newProps.push_back(0.0f); // nz
            // 计算并添加简单的 UV (例如根据坐标映射)
            float u = (mesh.vertProperties[i] + 5.0f) / 10.0f;
            float v = (mesh.vertProperties[i + 1] + 5.0f) / 10.0f;
            newProps.push_back(u);
            newProps.push_back(v);
        }
        mesh.vertProperties.resize(newProps.size(), 0.0f);
        for (size_t i = 0; i < newProps.size(); i++) {
            mesh.vertProperties[i] = newProps[i];
        }
        manifold::Manifold boxWithUV(mesh);

        manifold::Manifold ball = manifold::Manifold::Sphere(7, 32);

        // 使用布尔运算符
        manifold::Manifold intersected = boxWithUV + ball; // '^' 为交集, '+' 为并集, '-' 为差集

        // 导出为网格数据
        auto mesh_last = intersected.GetMeshGL(3);

        // auto entity    = object_3d_model("manifold ", mesh_last, {0, 0, -50});
        // uint32_t index = 7;
        // set_render_parameter(entity, "samplerColor", index);
    }
}
