//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_COMPONENT_H
#define HELLO_MAC_COMPONENT_H
#include <iostream>
#include <vector>
#include <memory>
#include <string>

// 前置声明：宿主类
class Actor;

// ===================== 组件基类（模拟 UActorComponent） =====================
class ActorComponent {
public:
    // 构造函数：传入宿主指针
    ActorComponent(Actor *owner, const std::string &compName)
        : Owner(owner), ComponentName(compName), bIsActive(true) {
        std::cout << "组件 [" << ComponentName << "] 已创建" << std::endl;
    }

    // 虚析构函数：支持子类多态
    virtual ~ActorComponent() {
        std::cout << "组件 [" << ComponentName << "] 已销毁" << std::endl;
    }

    // 组件初始化（生命周期函数）
    virtual void Initialize() {
        if (bIsActive) {
            std::cout << "组件 [" << ComponentName << "] 已初始化" << std::endl;
        }
    }

    // // 组件帧更新（生命周期函数）
    // virtual void Tick(float DeltaTime) {
    //     if (bIsActive) {
    //         std::cout << "组件 [" << ComponentName << "] 正在更新（DeltaTime：" << DeltaTime << "）" << std::endl;
    //     }
    // }

    // 激活/禁用组件
    void SetActive(bool bActive) {
        bIsActive = bActive;
        std::cout << "组件 [" << ComponentName << "] " << (bActive ? "已激活" : "已禁用") << std::endl;
    }

    // 获取组件名称
    std::string GetComponentName() const {
        return ComponentName;
    }

protected:
    Actor *Owner;              // 组件所属宿主（不可为空）
    std::string ComponentName; // 组件名称
    bool bIsActive;            // 组件是否激活
};


#endif //HELLO_MAC_COMPONENT_H
