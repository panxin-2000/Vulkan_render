//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_LOGIC_RENDER_DATA_H
#define HELLO_MAC_LOGIC_RENDER_DATA_H

#include <map>

#include "APP_utility_mixins.h"
#include "shader_common.h"
#include <type_traits>
#include "Texture_logic.h"
#include "utility.h"


/**
 * vertices_changed         <br>
 * indices_changed          <br>
 * texture_path_changed     <br>
 * texture_name_changed     <br>
 * vertex_path_changed      <br>
 * fragment_path_changed    <br>
 * geometry_path_changed    <br>
 * primitive_type_changed   <br>
 * uniform_buffer_changed   <br>
 */
enum status_change : uint16_t {
    no_change              = 0,
    vertices_changed       = 1 << 0,
    indices_changed        = 1 << 1,
    texture_path_changed   = 1 << 2,
    texture_name_changed   = 1 << 3,
    vertex_path_changed    = 1 << 4,
    fragment_path_changed  = 1 << 5,
    geometry_path_changed  = 1 << 6,
    primitive_type_changed = 1 << 7,
    uniform_buffer_changed = 1 << 8,
};

ENABLE_BITWISE_OPERATORS(status_change)


class logic_render_data : public NonCopyable {
#define add_mutex std::lock_guard<std::mutex> lock(mtx);

private:
    mutable std::mutex mtx;

public:
    std::string mesh_path_;
    std::vector<vertex_and_attributes> vertex_and_attributes_;
    std::string debug_name;
    Indices_type indices_;
    GPUPrimType prim_type_ = GPU_PRIM_TRIS;
    status_change status_  = no_change;

    // material 相关的内容
    std::vector<Texture_logic> textures;
    std::string texture_path_;
    std::string texture_name_;
    std::string vertexPath_;
    std::string geometryPath_;
    std::string fragmentPath_;
    draw_need_vk *proxy;

    logic_render_data() = default;

    ~logic_render_data() = default;

    void set_prim_type(const GPUPrimType prim_type) {
        prim_type_ = prim_type;
    }

    void push_vertex_and_attributes(vertex_and_attributes temp) {
        vertex_and_attributes_.push_back(temp);
    }


    void set_status_change(const status_change status) {
        status_ = status_ | status;
        // update_object_to_render(this);
    }

    status_change get_status_change() const {
        return status_;
    }


    auto get_indices() const {
        return indices_;
    }

    void set_indices(Indices_type indices) {
        add_mutex;
        indices_ = std::move(indices);
    }

    void set_texture(const std::string &path, const std::string &texture_name) {
        add_mutex;
        Texture_logic temp;
        temp.texture_type_ = texture_2d;
        temp.set_path(path, texture_name);
        textures.push_back(temp);
    }

    void set_vertex_shader(const std::string &path) {
        add_mutex;
        vertexPath_ = path;
    }

    void set_fragment_shader(const std::string &path) {
        add_mutex;
        fragmentPath_ = path;
    }

    void set_geometry_shader(const std::string &path) {
        add_mutex;
        geometryPath_ = path;
    }
#undef add_mutex
};


#endif //HELLO_MAC_LOGIC_RENDER_DATA_H
