//
// Created by 潘鑫 on 2025/12/25.
//

#ifndef HELLO_MAC_SCENE_COMPONENT_H
#define HELLO_MAC_SCENE_COMPONENT_H
#include "component.h"


class Scene_Component : public Actor_component {
private:
    std::vector<Actor *> parent;
    std::vector<Actor *> children;

public:
    Scene_Component(Actor *owner, const std::string &compName)
        : Actor_component(owner, compName) {
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
