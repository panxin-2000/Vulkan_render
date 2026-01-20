//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_LOGIC_RENDER_DATA_H
#define HELLO_MAC_LOGIC_RENDER_DATA_H

#include <map>

#include "APP_utility_mixins.h"
#include "shader_common.h"


#include <type_traits>

#include "texture_TBO.h"
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
    vertices_changed = 1 << 0,
    indices_changed = 1 << 1,
    texture_path_changed = 1 << 2,
    texture_name_changed = 1 << 3,
    vertex_path_changed = 1 << 4,
    fragment_path_changed = 1 << 5,
    geometry_path_changed = 1 << 6,
    primitive_type_changed = 1 << 7,
    uniform_buffer_changed = 1 << 8,
};

ENABLE_BITWISE_OPERATORS(status_change)


bool add_object_to_render(logic_render_data *render_object);

bool update_object_to_render(logic_render_data *render_object);

bool clean_object_to_render(logic_render_data *render_object);


class logic_render_data : public NonCopyable {
    struct vertex_and_attributes {
        Vertices_type vertices_;
        std::vector<VertexAttrib> vertex_attribs;
    };

public:
    std::vector<Texture_TBO> textures;
    std::vector<vertex_and_attributes> vertex_and_attributes;
    std::string debug_name;
    mutable std::mutex mtx;
    Vertices_type vertices_;
    Indices_type indices_;
    std::vector<VertexAttrib> vertex_attribs;
    std::string texture_path_;
    std::string texture_name_;
    std::string vertexPath_;
    std::string fragmentPath_;
    std::string geometryPath_;
    GPUPrimType prim_type_;
    std::map<std::string, std::tuple<Uniforms_type, data_value_or_ptr, uint8_t> > uniforms_map;
    status_change status_;


    logic_render_data() {
        // p_render_component = new render_component;
    }

    ~logic_render_data() {
    }


    void set_status_change(const status_change status) {
        status_ = status_ | status;
        update_object_to_render(this);
    }

    status_change get_status_change() const {
        return status_;
    }


#define add_mutex std::lock_guard<std::mutex> lock(mtx);

    void set_vertices(std::shared_ptr<std::vector<Point_3> > vertices) {
        add_mutex;
        vertices_ = std::move(vertices);
    }

    auto get_vertices() const {
        return vertices_;
    }

    auto get_indices() const {
        return indices_;
    }

    void set_indices(std::shared_ptr<std::vector<unsigned int> > indices) {
        add_mutex;
        indices_ = std::move(indices);
    }

    void set_texture(const std::string &path, const std::string &texture_name) {
        add_mutex;
        Texture_TBO temp;
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


    void add_uniform(const std::string &name, Uniforms_type uniforms_type, data_value_or_ptr &data,
                     uint8_t number = 1) {
        add_mutex;
        auto [it, success] =
                uniforms_map.insert({name, std::make_tuple(uniforms_type, data, number)});
        if (!success) {
            it->second = std::make_tuple(uniforms_type, data, number);
        }
    }
};


#endif //HELLO_MAC_LOGIC_RENDER_DATA_H
