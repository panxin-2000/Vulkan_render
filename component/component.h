//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_COMPONENT_H
#define HELLO_MAC_COMPONENT_H
#include <iostream>
#include <memory>
#include <string>

// 前置声明：宿主类
class Actor;

// ===================== 组件基类（模拟 UActorComponent） =====================
class Actor_component {
public:
    Actor_component(Actor *owner, const std::string &comp_name)
        : owner(owner), component_name(comp_name), bIsActive(true) {
        std::cout << "组件 [" << component_name << "] 已创建" << std::endl;
    }

    virtual ~Actor_component() {
        std::cout << "组件 [" << component_name << "] 已销毁" << std::endl;
    }

    virtual void initialize() {
        if (bIsActive) {
            std::cout << "组件 [" << component_name << "] 已初始化" << std::endl;
        }
    }

    void set_active(bool bActive) {
        bIsActive = bActive;
        std::cout << "组件 [" << component_name << "] " << (bActive ? "已激活" : "已禁用") << std::endl;
    }

    // 获取组件名称
    std::string get_name() const {
        return component_name;
    }

    Actor *get_owner() const {
        return owner;
    }

protected:
    Actor *owner;               // 组件所属宿主（不可为空）
    std::string component_name; // 组件名称
    bool bIsActive;             // 组件是否激活
};


#endif //HELLO_MAC_COMPONENT_H
