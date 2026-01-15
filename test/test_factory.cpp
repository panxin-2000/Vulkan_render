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
class Document {
public:
    virtual ~Document() = default;

    virtual void open() = 0;
};

// 2. 具体产品
class PdfDocument : public Document {
public:
    void open() override { std::cout << "打开 PDF..." << std::endl; }
};

class WordDocument : public Document {
public:
    void open() override { std::cout << "打开 Word..." << std::endl; }
};

// 3. 统一工厂类（使用注册制）
class DocumentFactory {
public:
    // 定义一个函数类型，用于创建对象
    using Creator = std::function<std::unique_ptr<Document>()>;

    // 核心：静态注册表
    static DocumentFactory &instance() {
        static DocumentFactory factory;
        return factory;
    }

    // 注册新产品类型
    void registerType(const std::string &name, Creator creator) {
        registry[name] = creator;
    }

    // 根据字符串创建对象
    std::unique_ptr<Document> create(const std::string &name) {
        if (registry.find(name) != registry.end()) {
            return registry[name](); // 调用注册好的构造函数
        }
        return nullptr;
    }

private:
    std::map<std::string, Creator> registry;

    DocumentFactory() = default; // 单例模式
};

// 4. 使用示例
TEST(factory, factory_method) {
    // 在程序初始化时注册产品（这部分可以放在各自的实现文件中）
    DocumentFactory::instance().registerType("pdf", []() {
        return std::make_unique<PdfDocument>();
    });
    DocumentFactory::instance().registerType("word", []() {
        return std::make_unique<WordDocument>();
    });

    // 业务逻辑：只需传递字符串
    std::string userInput = "pdf";
    auto doc = DocumentFactory::instance().create(userInput);

    if (doc) {
        doc->open();
    } else {
        std::cout << "不支持的文档格式" << std::endl;
    }
}
