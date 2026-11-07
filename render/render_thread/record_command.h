//
// Created by 潘鑫 on 2026/9/17.
//

#ifndef HELLO_MAC_LOGIC_TO_RENDER_H
#define HELLO_MAC_LOGIC_TO_RENDER_H

#define ALIGN_16(size) (((size) + 15) & ~15)


#include <tuple>
#include <utility>

// ==========================================
// 1. 基础命令头（类型擦除的关键）
// ==========================================
struct RenderCommandHeader {
    // 统一的跳板函数指针：输入当前命令的起始内存地址，内部负责解包执行
    void (*execute)(void *self);

    // 下一条命令在缓冲区中的偏移量（用于步进式链表读取）
    void *next_offset;
};

// ==========================================
// 2. 泛型命令包装器（将函数和参数强类型化）
// ==========================================
template<typename F, typename... Args>
struct ConcreteRenderCommand {
    RenderCommandHeader header;
    F func;                   // 存储可调用对象（函数指针或 Lambda）
    std::tuple<Args...> args; // 紧凑存储所有参数数据

    // 静态跳板函数：将裸指针转回具体类型，展开 tuple 并调用函数
    static void Dispatch(void *raw_self) {
        auto self = static_cast<ConcreteRenderCommand *>(raw_self);
        std::apply(self->func, self->args);
        self->~ConcreteRenderCommand(); // 删除在这里
    }
};


#endif //HELLO_MAC_LOGIC_TO_RENDER_H
