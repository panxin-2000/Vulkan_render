//
// Created by 潘鑫 on 2025/12/25.
//

#ifndef HELLO_MAC_SCENE_COMPONENT_H
#define HELLO_MAC_SCENE_COMPONENT_H
#include <iostream>
#include <vector>
#include <entt/entt.hpp>


class Scene_Component {
private:
    std::vector<entt::entity> parent;
    std::vector<entt::entity> children;

public:
    Scene_Component() {
    }


    // 重写初始化：模拟加载模型
    void Initialize() override {
        if (bIsActive) {
            std::cout << "场景组件 [" << component_name << "] 已加载模型" << std::endl;
        }
    }

    // 自定义功能：设置模型可见性
    void set_visibility(bool bVisible) {
        std::cout << "场景组件 [" << component_name << "] " << (bVisible ? "显示" : "隐藏") << "模型" << std::endl;
    }
};

#endif //HELLO_MAC_SCENE_COMPONENT_H
