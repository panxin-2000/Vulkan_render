#include <iostream>
#include <string>
#include <memory>

// 抽象产品 A: 按钮
class Button {
public:
    virtual ~Button() {
    }

    virtual void paint() const = 0;
};

// 抽象产品 B: 文本框
class TextField {
public:
    virtual ~TextField() {
    }

    virtual void render() const = 0;
};

// 具体产品
// Windows 风格组件
class WinButton : public Button {
public:
    WinButton() {
        std::cout << "创建 Windows 风格按钮\n";
    }

    void paint() const override { std::cout << "绘制 Windows 风格按钮\n"; }
};

class WinTextField : public TextField {
public:
    WinTextField() {
        std::cout << "创建 Windows 风格文本框\n";
    }

    void render() const override { std::cout << "绘制 Windows 风格文本框\n"; }
};

// macOS 风格组件
class MacButton : public Button {
public:
    MacButton() {
        std::cout << "创建 mac 风格按钮\n";
    }

    void paint() const override { std::cout << "绘制 macOS 风格按钮\n"; }
};

class MacTextField : public TextField {
public:
    MacTextField() {
        std::cout << "创建 mac 风格文本框\n";
    }

    void render() const override { std::cout << "绘制 macOS 风格文本框\n"; }
};

// 抽象工厂
class GUIFactory {
public:
    virtual std::unique_ptr<Button> createButton() const = 0;

    virtual std::unique_ptr<TextField> createTextField() const = 0;

    virtual ~GUIFactory() {
    }
};

// 具体工厂
class WinFactory : public GUIFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<WinButton>();
    }

    std::unique_ptr<TextField> createTextField() const override {
        return std::make_unique<WinTextField>();
    }
};

class MacFactory : public GUIFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<MacButton>();
    }

    std::unique_ptr<TextField> createTextField() const override {
        return std::make_unique<MacTextField>();
    }
};

class UI_block {
public:
    const GUIFactory &factory;
    std::unique_ptr<Button> btn;
    std::unique_ptr<TextField> text;

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

TEST(absfactory, absfactory) {
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
