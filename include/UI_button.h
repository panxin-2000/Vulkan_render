//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_UI_BUTTON_H
#define HELLO_MAC_UI_BUTTON_H


#include "render_component.h"
#include "input_component.h"
#include "base_event.h"
#include "ECS.h"
#include "entity_name_component.h"
#include "observer_manage.h"
#include "render_object_manage.h"

class UI_button {
private:
    entt::entity entity_;

public:
    UI_button(const std::string &name, entt::entity entity) {
        std::cout << "Labyrinth::Labyrinth()" << std::endl;


        /***************创建*******************/
        entity_ = entity;
        get_entt_instance().emplace<render_component>(entity);
        get_entt_instance().emplace<Input_Component>(entity, on_Event);

        get_entt_instance().emplace<Position_component>(entity);
        get_entt_instance().emplace<Drag_event>(entity);
        get_entt_instance().emplace<Name_component>(entity, name);


        auto &render = get_entt_instance().get<render_component>(entity);

        /***************设置参数**********************/
        std::vector<VertexAttrib> vertex_attribs;
        vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE, sizeof(Point_3), (void *) 0);
        // vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

        std::vector<Point_3> vertices;
        std::vector<unsigned int> indices;
        int min_x = 0;
        int min_y = 0;
        int max_x = 800;
        int max_y = 600;
        indices.push_back(vertices.size() + 0);
        indices.push_back(vertices.size() + 1);
        indices.push_back(vertices.size() + 2);
        indices.push_back(vertices.size() + 2);
        indices.push_back(vertices.size() + 3);
        indices.push_back(vertices.size() + 0);
        vertices.emplace_back(min_x, min_y, 0); //0 1 2
        vertices.emplace_back(max_x, min_y, 0);
        vertices.emplace_back(max_x, max_y, 0); // 2 3 0
        vertices.emplace_back(min_x, max_y, 0);
        // 参数这里最重要的是下面的两行

        // 参数这里最重要的是下面的两行
        render.set_VBO_parameter(vertices.size() * sizeof(Point_3), vertices.data(), vertex_attribs);
        render.set_EBO_parameter(indices.size() * sizeof(GLuint), indices.data(), indices.size());
        render.set_vertex_shader("render/shader/different_color.vert");
        render.set_fragment_shader("render/shader/different_color.frag");

        auto &position = get_entt_instance().get<Position_component>(entity_);
        position.update_2D_position_matrix();

        // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
        /***************添加到渲染管理器**********************/
        add_object_to_render_manager(&render);
        add_render_windows();  // 为什么一定是要在这里？ 难道全局实例化是有问题的

    };


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
                if (auto *position = get_entt_instance().try_get<Position_component>(entity_)) {
                    position->set_zoom(event);
                    position->update_position();
                }
                break;
            case MOUSE_MOVE:

                break;
            default:
                break;
        }
    }
};

#endif //HELLO_MAC_UI_BUTTON_H
