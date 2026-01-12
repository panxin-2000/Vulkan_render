//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_ACTOR_H
#define HELLO_MAC_ACTOR_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include "Position_component.h"
#include "render_component.h"
#include "input_component.h"   // 只是把这个向后移动就没有问题了


class Actor_component;
class render_component;
class Position_component;
class Input_Component;


class Actor {
public:
    Actor(const std::string &actorName) : actor_name(actorName) {
        std::cout << "宿主 [" << actor_name << "] 已创建" << std::endl;
    }

    ~Actor() {
        // 销毁所有组件（与宿主生命周期同步）
        components.clear();
        std::cout << "宿主 [" << actor_name << "] 已销毁，所有组件已清理" << std::endl;
    }


    render_component *add_render_component() {
        render = new render_component(this, "render");
        render->initialize();
        return render;
    }

    render_component *get_render_component() {
        return render;
    }


    Position_component *add_position_component() {
        position = new Position_component(this, "render");
        position->initialize();
        return position;
    }

    Position_component *get_position_component() {
        return position;
    }

    Input_Component *add_input_component() {
        input = new Input_Component(this, "input");
        // input->initialize();
        return input;
    }

    Input_Component *get_input_component() {
        return input;
    }

    std::string get_name() const {
        return actor_name;
    }

    // 根据组件名称查找组件
    Actor_component *find_component(const std::string &comp_name) {
        for (const auto &comp: components) {
            if (comp->get_name() == comp_name) {
                return comp.get();
            }
        }
        std::cout << "未找到组件 [" << comp_name << "]" << std::endl;
        return nullptr;
    }

private:
    std::string actor_name; // 宿主名称
    // 组件列表：用 shared_ptr 管理组件生命周期，自动释放内存
    render_component *render = nullptr;
    Position_component *position = nullptr;
    Input_Component *input = nullptr;
    std::vector<std::shared_ptr<Actor_component> > components;
};

#endif //HELLO_MAC_ACTOR_H
