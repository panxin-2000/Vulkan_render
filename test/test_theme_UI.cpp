//
// Created by 潘鑫 on 2026/1/15.
//
#include <iostream>
#include <string>
#include <memory>
#include "set_graph.h"
#include <entt/entt.hpp>
#include "ECS.h"

// 1. 定义皮肤/主题数据结构（代替具体产品类）
struct UIStyleConfig {
    std::string buttonTexture;
    std::string fontPath;
    float borderRadius;
};

// 抽象工厂
class GUIFactory {
public:
    UIStyleConfig theme; // 可以设置为全局 theme

    bool GUIFactory_init(UIStyleConfig config) {
        theme = config;
    }


    entt::entity createButton() const {
        auto entity = get_entt_instance().create();

        // 将不同系统的风格也作为一个组件
        // auto e = reg.create();
        // reg.emplace<ButtonTag>(e);
        // reg.emplace<Label>(e, text);
        // // 直接从 theme 数据中提取“具体产品”的特征
        // reg.emplace<RenderData>(e, theme.buttonTexture);
        // reg.emplace<StyleData>(e, theme.borderRadius);

        return entity;
    }


    entt::entity createTextField() const {
        auto entity = get_entt_instance().create();
        return entity;
    }

    ~GUIFactory() {
    }


    std::map<std::string, UIStyleConfig> registry; // 同样的，这里可以在全局其他位置存储
};

UIStyleConfig get_global_config() {
    UIStyleConfig config;
    return config;
}

class UI_block {
public:
    GUIFactory factory;

    UI_block() {
        factory.GUIFactory_init(get_global_config());
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
    UI_block win_app;
    win_app.createButton();
    win_app.createTextField();

    std::cout << "\n--- 运行 macOS 环境 ---\n";
    UI_block mac_app;
    mac_app.createButton();
    mac_app.createTextField();

    return 0;
}
