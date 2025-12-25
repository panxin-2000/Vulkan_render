//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H
#include "component.h"


class RenderComponent : public ActorComponent {
public:
    RenderComponent(Actor *owner, const std::string &compName)
        : ActorComponent(owner, compName) {
    }

    // 重写初始化：模拟加载模型
    void Initialize() override {
        if (bIsActive) {
            std::cout << "渲染组件 [" << ComponentName << "] 已加载模型" << std::endl;
        }
    }

    // // 重写 Tick：模拟更新渲染状态
    // void Tick(float DeltaTime) override {
    //     if (bIsActive) {
    //         std::cout << "渲染组件 [" << ComponentName << "] 同步渲染状态（DeltaTime：" << DeltaTime << "）" << std::endl;
    //     }
    // }

    // 自定义功能：设置模型可见性
    void SetVisibility(bool bVisible) {
        std::cout << "渲染组件 [" << ComponentName << "] " << (bVisible ? "显示" : "隐藏") << "模型" << std::endl;
    }
};

#endif //HELLO_MAC_RENDER_COMPONENT_H
