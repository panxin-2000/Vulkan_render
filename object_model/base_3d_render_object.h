//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_3D_MODEL_DISPLAY_H
#define HELLO_MAC_3D_MODEL_DISPLAY_H

#include "global_singleton.h"
#include "shader_component.h"
#include "base_geometry/base.h"
#include "manifold/manifold.h"
#include "base_render_object.h"

class object_3d : public logic_render_object {
public:
    object_3d(const std::string &name);

    object_3d &object_3d_add_mesh(const AABB_min_max<Point_3> &bounding_box);

    object_3d &add_manifold_mesh(manifold::MeshGL &mesh);

    object_3d &add_mesh(const std::string &mesh_path);

    object_3d &add_mesh(const AABB_min_max<Point_3> &bounding_box);
    object_3d &add_postprocess();

    object_3d &add_sky_box();

    object_3d &set_transform(const Eigen::Vector3f offset     = Eigen::Vector3f::Zero(),
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity());

    template<typename T1>
    object_3d &add_render_parameter(const std::string &binding_name, T1 &binding_data) {
        set_render_parameter(entity, binding_name, binding_data);
        return *this;
    }

    template<typename T1>
    object_3d &add_push_constant_parameter(const std::string &binding_name, T1 &binding_data) {
        set_push_constant_parameter(entity, binding_name, binding_data);
        return *this;
    }
};

inline object_3d create_object_3d(const std::string &name) {
    return object_3d(name);
}


entt::entity object_line_old(const std::string &name);

#endif //HELLO_MAC_3D_MODEL_DISPLAY_H
