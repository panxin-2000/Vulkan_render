//
// Created by 潘鑫 on 2026/1/15.
//
#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <functional>

#include "gtest/gtest.h"

// 1. 产品基类
class button {
public:
    virtual ~button() = default;

    virtual void draw() = 0;
};

// 2. 具体产品
class mac_button : public button {
public:
    mac_button() : button() {
        std::cout << "创建 [macOS 风格] 柔和圆角按钮\n";
    }

    void draw() override {
        std::cout << "绘制 [macOS 风格] 柔和圆角按钮\n";
    }
};

class win_button : public button {
public:
    win_button() : button() {
        std::cout << "创建 [Windows 风格] 扁平化按钮\n";
    }


    void draw() override {
        std::cout << "绘制 [Windows 风格] 柔和圆角按钮\n";
    }
};

// 3. 统一工厂类（使用注册制）
class buttonFactory {
public:
    // 定义一个函数类型，用于创建对象
    using Creator = std::function<std::unique_ptr<button>()>;

    // 核心：静态注册表
    static buttonFactory &instance() {
        static buttonFactory factory;
        return factory;
    }

    // 注册新产品类型
    void registerType(const std::string &name, Creator creator) {
        registry[name] = creator;
    }

    // 根据字符串创建对象
    std::unique_ptr<button> create(const std::string &name) {
        if (registry.find(name) != registry.end()) {
            return registry[name](); // 调用注册好的构造函数
        }
        return nullptr;
    }

private:
    std::map<std::string, Creator> registry;

    buttonFactory() = default; // 单例模式
};

// 4. 使用示例
TEST(factory, factory_method) {
    // 在程序初始化时注册产品（这部分可以放在各自的实现文件中）
    buttonFactory::instance().registerType("mac", []() {
        return std::make_unique<mac_button>();
    });
    buttonFactory::instance().registerType("win", []() {
        return std::make_unique<win_button>();
    });

    // 业务逻辑：只需传递字符串
    std::string userInput = "mac";
    auto button = buttonFactory::instance().create(userInput);

    if (button) {
        button->draw();
    } else {
        std::cout << "不支持的文档格式" << std::endl;
    }
}
