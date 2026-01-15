//
// Created by 潘鑫 on 2026/1/15.
//
#include <iostream>
#include <string>
#include <memory>
#include "set_graph.h"
#include <entt/entt.hpp>
#include "ECS.h"


// 抽象工厂
class GUIFactory {
public:
    virtual entt::entity createButton() const = 0;

    virtual entt::entity createTextField() const = 0;

    virtual ~GUIFactory() {
    }
};

// 具体工厂
class WinFactory : public GUIFactory {
public:
    entt::entity createButton() const {
        auto entity = get_entt_instance().create();

        return entity;
    }

    entt::entity createTextField() const {
        auto entity = get_entt_instance().create();

        return entity;
    }
};

class MacFactory : public GUIFactory {
public:
    entt::entity createButton() const {
        auto entity = get_entt_instance().create();

        return entity;
    }

    entt::entity createTextField() const {
        auto entity = get_entt_instance().create();

        return entity;
    }
};

class UI_block {
public:
    const GUIFactory &factory;

    UI_block(const GUIFactory &factory) : factory(factory) {
    };

    auto createButton() {
        auto btn = factory.createButton();
        return btn;
    }

    auto createTextField() {
        auto btn = factory.createTextField();
        return btn;
    }
};

int main() {
    // 实际应用中，可以根据操作系统环境变量自动选择工厂
    std::cout << "--- 运行 Windows 环境 ---\n";
    WinFactory winFactory;
    UI_block win_app(winFactory);
    win_app.createButton();
    win_app.createTextField();

    std::cout << "\n--- 运行 macOS 环境 ---\n";
    MacFactory macFactory;
    UI_block mac_app(macFactory);
    mac_app.createButton();
    mac_app.createTextField();

    return 0;
}
