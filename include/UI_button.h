//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_UI_BUTTON_H
#define HELLO_MAC_UI_BUTTON_H


#include "Render_thread_data.h"
#include "input_component.h"
#include "base_event.h"
#include "ECS.h"
#include "entity_name_component.h"
#include "model_matrix_component.h"
#include "observer_manage.h"
#include "render_object_manage.h"

class UI_button {
private:
    entt::entity entity_;

public:
    UI_button(const std::string &name, entt::entity entity,
              float min_x = 20,
              float min_y = 20,
              float max_x = 30,
              float max_y = 30) {
        std::cout << "UI_button" << std::endl;


        /***************创建*******************/
        entity_ = entity;
        get_entt_instance().emplace<logic_render_data>(entity);
        get_entt_instance().emplace<Input_Component>(entity, on_Event);

        get_entt_instance().emplace<Scene_Component>(entity);
        if (auto *scene_node = get_entt_instance().try_get<Scene_Component>(entity)) {
            scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
        }
        get_entt_instance().emplace<Drag_event>(entity);
        get_entt_instance().emplace<Name_component>(entity, name);


        auto &render = get_entt_instance().get<logic_render_data>(entity);

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
        render.debug_name = name;
        render.set_vertices(vertices);
        render.set_indices(indices);
        render.vertex_attribs = vertex_attribs;
        render.set_vertex_shader("render/shader/different_color.vert");
        render.set_fragment_shader("render/shader/different_color.frag");

        auto &position = get_entt_instance().get<Scene_Component>(entity_);
        position.update_2D_position_matrix();

        add_object_to_render(&render);

        // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
        /***************添加到渲染管理器**********************/
    };


    static bool on_Event(entt::entity entity_, const base_event_with_stamp &event) {
        auto temp_type = event.event_type;
        switch (temp_type) {
            case EVT_KEY_A:

                break;
            case EVT_KEY_Q:

                break;
            case MOUSE_LEFT:
                std::cout << "MOUSE_LEFT" << std::endl;
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
                    std::cout << "UI_button MOUSE_LEFT" << std::endl;
                    scene_node->set_position_offset(event);
                    scene_node->update_2D_position_matrix();
                    if (auto *render = get_entt_instance().try_get<logic_render_data>(entity_)) {
                        render->set_status_change(uniform_buffer_changed);
                    }
                }
            default:
                break;
        }
    }
};

#endif //HELLO_MAC_UI_BUTTON_H
