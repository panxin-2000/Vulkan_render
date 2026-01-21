//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_UI_BUTTON_H
#define HELLO_MAC_UI_BUTTON_H


#include "Render_thread_data.h"
#include "input_component.h"
#include "base_event.h"
#include "entity_name_component.h"
#include "global_singleton.h"
#include "model_matrix_component.h"
#include "observer_manage.h"
#include "render_object_manage.h"

entt::entity UI_button(const std::string &name,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y);

// 按键和鼠标有两种截然不同的策略，基本上，所以的鼠标的点击时并没有反应，但是呢？
// 松开时 采取执行按键设计的动作

// 键盘是另一种操作，按下时就去执行响应的动作，有时会增加弹窗来进行确认

static wmOperatorStatus on_Event(entt::entity entity_, const base_event_with_stamp &event) {
    auto temp_type = event.event_type;
    switch (temp_type) {
        case EVT_KEY_X:
            // 删除当前鼠标位置的元素
            if (event.event_code == KM_PRESS)
                if (g_entt().valid(entity_)) {
                    g_entt().emplace_or_replace<PendingDestroyTag>(entity_);
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        case EVT_KEY_ESCAPE:
            if (event.event_code == KM_PRESS) {
                std::cout << " button  EVT_KEY_ESCAPE KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态 先用按下的状态，之后再更改
                return OPERATOR_CANCELLED;
            }
            break;
        case MOUSE_LEFT:
            if (event.event_code == KM_PRESS) {
                std::cout << " button  MOUSE_LEFT KM_PRESS" << std::endl;
                // 需要增加模态的处理 返回锁定模态
                return OPERATOR_RUNNING_MODAL;
            }
            if (event.event_code == KM_RELEASE) {
                std::cout << " button  MOUSE_LEFT KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态
                auto block_entity = UI_button("新按钮", 10, 10, 220, 220);
                return OPERATOR_FINISHED;
            }
            break;
        case MOUSE_RIGHT:
            break;
        case WHEEL_UP_MOUSE:
            if (auto *scene_node = g_entt().try_get<Scene_Component>(entity_)) {
                scene_node->set_zoom(event);
                scene_node->update_position();
            }
            break;
        case MOUSE_MOVE:
            if (auto *scene_node = g_entt().try_get<Scene_Component>(entity_)) {
                if (g_entt().all_of<Scene_Component>(entity_)) {
                    auto &name = g_entt().get<Name_component>(entity_);
                    // std::cout << "UI_button" << name.name << " MOUSE_MOVE" << std::endl;
                }
                scene_node->set_position_offset(event);
                scene_node->update_2D_position_matrix();
                if (auto *render = g_entt().try_get<logic_render_data *>(entity_)) {
                    (*render)->set_status_change(uniform_buffer_changed);
                }
                // 包围盒的位置还需要同步更新
                return OPERATOR_RUNNING_MODAL;
            }
            break;
        default:
            return OPERATOR_PASS_THROUGH;
    }
    return OPERATOR_HANDLED;
}


entt::entity UI_button(const std::string &name,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y) {
    std::cout << "UI_button" << std::endl;
    std::string_view df = "";

    LOG_INFO(g_log(), "UI create  {} {} {} {} {} ", name, min_x, min_y, max_x, max_y);

    entt::entity entity_ = g_entt().create();

    /***************创建*******************/
    g_entt().emplace<logic_render_data *>(entity_, new logic_render_data);
    g_entt().emplace<Input_Component>(entity_, on_Event);

    g_entt().emplace<Scene_Component>(entity_);
    if (auto *scene_node = g_entt().try_get<Scene_Component>(entity_)) {
        scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
    }
    g_entt().emplace<Drag_event>(entity_);
    g_entt().emplace<Name_component>(entity_, name);


    if (g_entt().all_of<logic_render_data *>(entity_)) {
        auto render = g_entt().get<logic_render_data *>(entity_);

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
        vertex_and_attributes temp = {vertices, vertex_attribs};
        render->debug_name = name;
        render->push_vertex_and_attributes(temp);
        render->set_indices(indices);
        render->set_vertex_shader("render/shader/different_color.vert");
        render->set_fragment_shader("render/shader/different_color.frag");

        add_object_to_render(render); // 因为这里没有区分。全部都在场景的根节点之下
    }

    if (g_entt().all_of<Scene_Component>(entity_)) {
        auto &position = g_entt().get<Scene_Component>(entity_);
        position.update_2D_position_matrix();
        scene_root_add_child(entity_);
    }
    return entity_;

    // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
    /***************添加到渲染管理器**********************/
}

#endif //HELLO_MAC_UI_BUTTON_H
