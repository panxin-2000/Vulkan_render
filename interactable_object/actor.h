//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_ACTOR_H
#define HELLO_MAC_ACTOR_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
class ActorComponent;


class Actor {
public:
    Actor(const std::string &actorName) : ActorName(actorName) {
        std::cout << "宿主 [" << ActorName << "] 已创建" << std::endl;
    }

    ~Actor() {
        // 销毁所有组件（与宿主生命周期同步）
        Components.clear();
        std::cout << "宿主 [" << ActorName << "] 已销毁，所有组件已清理" << std::endl;
    }

    // ===================== 核心方法：Add Component（添加组件） =====================
    // 模板函数：支持任意组件子类，自动维护组件列表
    // 这里稍微有点意思
    template<typename T, typename... Args>
    T *AddComponent(Args &&... args) {
        // 1. 检查是否为 ActorComponent 子类
        static_assert(std::is_base_of<ActorComponent, T>::value, "T 必须是 ActorComponent 的子类！");

        // 2. 创建组件实例（智能指针管理内存，避免内存泄漏）
        std::shared_ptr<T> newComponent = std::make_shared<T>(this, std::forward<Args>(args)...);

        // 3. 将组件添加到宿主的组件列表
        Components.push_back(newComponent);

        // 4. 初始化组件
        newComponent->Initialize();

        // 5. 返回组件裸指针（供外部调用）
        return newComponent.get();
    }

    // // 宿主帧更新：触发所有组件的 Tick 方法
    // void Tick(float DeltaTime) {
    //     std::cout << "\n宿主 [" << ActorName << "] 开始更新" << std::endl;
    //     // 遍历所有组件，执行 Tick
    //     for (const auto &comp: Components) {
    //         comp->Tick(DeltaTime);
    //     }
    // }

    // // 根据组件名称查找组件
    // ActorComponent *FindComponentByName(const std::string &compName) {
    //     for (const auto &comp: Components) {
    //         if (comp->GetComponentName() == compName) {
    //             return comp.get();
    //         }
    //     }
    //     std::cout << "未找到组件 [" << compName << "]" << std::endl;
    //     return nullptr;
    // }

private:
    std::string ActorName; // 宿主名称
    // 组件列表：用 shared_ptr 管理组件生命周期，自动释放内存

    std::vector<std::shared_ptr<ActorComponent> > Components;
};

#endif //HELLO_MAC_ACTOR_H
