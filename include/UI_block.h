//
// Created by 潘鑫 on 2026/1/17.
//

#ifndef HELLO_MAC_UI_BLOCK_H
#define HELLO_MAC_UI_BLOCK_H


#include "Render_thread_data.h"
#include "input_component.h"
#include "base_event.h"
#include "ECS.h"
#include "entity_name_component.h"
#include "model_matrix_component.h"
#include "observer_manage.h"
#include "logic_render_data.h"
#include "scene_component.h"
#include "UI_button.h"

class UI_block {
private:
    entt::entity entity_;

public:
    UI_block(const std::string &name,
             float min_x,
             float min_y,
             float max_x,
             float max_y) {
        std::cout << "UI_block" << std::endl;
        entity_ = get_entt_instance().create();


        /***************创建*******************/
        get_entt_instance().emplace<logic_render_data>(entity_);
        get_entt_instance().emplace<Input_Component>(entity_, on_Event);
        get_entt_instance().emplace<Scene_Component>(entity_);
        if (auto *scene_node = get_entt_instance().try_get<Scene_Component>(entity_)) {
            scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
        }
        scene_root_add_child(entity_);
        get_entt_instance().emplace<Drag_event>(entity_);
        get_entt_instance().emplace<Name_component>(entity_, name); {
            auto &render = get_entt_instance().get<logic_render_data>(entity_);

            /***************设置参数**********************/
            std::vector<VertexAttrib> vertex_attribs;
            vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE);
            // vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

            auto vertices = std::make_shared<std::vector<Point_3> >();
            auto indices = std::make_shared<std::vector<unsigned int> >();
            // 要改这里，需要改的内容似乎就有点说了，之后再看看怎么改吧。

            indices->push_back(vertices->size() + 0);
            indices->push_back(vertices->size() + 1);
            indices->push_back(vertices->size() + 2);
            indices->push_back(vertices->size() + 2);
            indices->push_back(vertices->size() + 3);
            indices->push_back(vertices->size() + 0);
            vertices->emplace_back(min_x, min_y, 0); //0 1 2
            vertices->emplace_back(max_x, min_y, 0);
            vertices->emplace_back(max_x, max_y, 0); // 2 3 0
            vertices->emplace_back(min_x, max_y, 0);
            // 参数这里最重要的是下面的两行

            // 参数这里最重要的是下面的两行
            render.set_vertices(vertices);
            render.debug_name = name;
            render.set_indices(indices);
            render.vertex_attribs = vertex_attribs;
            render.set_vertex_shader("render/shader/different_color.vert");
            render.set_fragment_shader("render/shader/different_color.frag");

            auto &scene_node = get_entt_instance().get<Scene_Component>(entity_);
            scene_node.update_2D_position_matrix();

            add_object_to_render(&render);
        }

        // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
        /***************添加到渲染管理器**********************/
    };

    UI_button add_button(const std::string &name,
                         int min_x,
                         int min_y,
                         int max_x,
                         int max_y) {
        // auto &render = get_entt_instance().get<logic_render_data>(block_entity);
        // auto vertices = render.get_vertices();
        // auto indices = render.get_indices();
        //
        // indices->push_back(vertices->size() + 0);
        // indices->push_back(vertices->size() + 1);
        // indices->push_back(vertices->size() + 2);
        // indices->push_back(vertices->size() + 2);
        // indices->push_back(vertices->size() + 3);
        // indices->push_back(vertices->size() + 0);
        // vertices->emplace_back(min_x, min_y, 0); //0 1 2
        // vertices->emplace_back(max_x, min_y, 0);
        // vertices->emplace_back(max_x, max_y, 0); // 2 3 0
        // vertices->emplace_back(min_x, max_y, 0);
        auto entity = get_entt_instance().create();

        get_entt_instance().emplace<UI_button>(entity, name, entity, min_x, min_y, max_x, max_y);

        auto &parent_scene = get_entt_instance().get<Scene_Component>(entity_);
        auto &children_scene = get_entt_instance().get<Scene_Component>(entity);
        parent_scene.add_child(entity);
        children_scene.add_parent(entity);
    }


    static bool on_Event(entt::entity entity_, const base_event_with_stamp &event) {
        auto temp_type = event.event_type;
        switch (temp_type) {
            case EVT_KEY_A:

                break;
            case EVT_KEY_Q:

                break;
            case MOUSE_LEFT:
                std::cout << "MOUSE_LEFT" << std::endl; // 也是能够进入的，需要更改事件的分发
                break;
            case MOUSE_RIGHT:
                break;
            case WHEEL_UP_MOUSE:
                if (auto *scene_node = get_entt_instance().try_get<Scene_Component>(entity_)) {
                    scene_node->set_zoom(event);
                    scene_node->update_position();
                }
                break;
            case MOUSE_MOVE:
                if (auto *scene_node = get_entt_instance().try_get<Scene_Component>(entity_)) {
                    auto &name = get_entt_instance().get<Name_component>(entity_);
                    std::cout << "UI_button" << name.name << " MOUSE_LEFT" << std::endl;
                    scene_node->set_position_offset(event);
                    scene_node->update_2D_position_matrix();
                    // if (auto *render = get_entt_instance().try_get<logic_render_data>(entity_)) {
                    // render->set_status_change(uniform_buffer_changed);
                    // }
                }

                break;
            default:
                return false;
        }
        return true;
    }
};


#endif //HELLO_MAC_UI_BLOCK_H
