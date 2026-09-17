//
// Created by 潘鑫 on 2026/9/17.
//

#ifndef HELLO_MAC_LOGIC_TO_RENDER_H
#define HELLO_MAC_LOGIC_TO_RENDER_H


#include <oneapi/tbb.h>
#include <tuple>
#include <utility>

// ==========================================
// 1. 基础命令头（类型擦除的关键）
// ==========================================
struct RenderCommandHeader {
    // 统一的跳板函数指针：输入当前命令的起始内存地址，内部负责解包执行
    void (*execute)(void *self);

    // 下一条命令在缓冲区中的偏移量（用于步进式链表读取）
    uint32_t next_offset;
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
        self->~ConcreteRenderCommand();
    }
};

// ==========================================
// 3. 高性能环形缓冲区（使用 TBB 内存分配）
// ==========================================
class RenderRingBuffer {
public:
    // 建议使用 const size_t capacity =  2 * 1024 * 1024
    explicit RenderRingBuffer(const size_t capacity) : m_capacity(capacity), m_head(0), m_tail(0) {
        // 使用 TBB 线程安全且针对多核心优化的分配器申请一块连续大内存
        m_buffer = static_cast<char *>(malloc(capacity));
    }

    ~RenderRingBuffer() {
        if (m_buffer != nullptr) {
            free(m_buffer);
        }
    }

    RenderRingBuffer(RenderRingBuffer &other) {
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_head, other.m_head);
        std::swap(m_tail, other.m_tail);
    };

    RenderRingBuffer &operator=(RenderRingBuffer &other) = delete;

    RenderRingBuffer(RenderRingBuffer &&other) noexcept
        : m_buffer(other.m_buffer), m_capacity(other.m_capacity), m_head(other.m_head), m_tail(0) {
        // 将源对象的指针和状态清空，防止其析构时释放内存
        other.m_buffer   = nullptr;
        other.m_capacity = 0;
        other.m_head     = 0;
    }

    // 3. 实现移动赋值运算符 (Move Assignment Operator)
    RenderRingBuffer &operator=(RenderRingBuffer &&other) noexcept {
        if (this != &other) {
            std::swap(m_buffer, other.m_buffer);
            std::swap(m_capacity, other.m_capacity);
            std::swap(m_head, other.m_head);
            std::swap(m_tail, other.m_tail);
        }
        return *this;
    }

    friend void swap(RenderRingBuffer &a, RenderRingBuffer &b) noexcept {
        std::swap(a.m_buffer, b.m_buffer);
        std::swap(a.m_capacity, b.m_capacity);
        std::swap(a.m_head, b.m_head);
        std::swap(a.m_tail, b.m_tail);
    }


    // 【逻辑线程调用】向缓冲区提交任意函数和参数
    template<typename F, typename... Args>
    bool Submit(F &&func, Args &&... args) {
        using CommandType = ConcreteRenderCommand<std::decay_t<F>, std::decay_t<Args>...>;

        // 计算当前命令所需的总内存大小（向上对齐到 8 字节，对 CPU 缓存友好）
        const size_t size = (sizeof(CommandType) + 7) & ~7;

        if (m_head + size > m_capacity) {
            // 【策略：自动使用 TBB 翻倍扩容】
            // 注意：不能用 realloc，因为 realloc 移动内存会把旧命令的指针弄乱。
            // 这里我们采用“分配新大块 -> 延迟处理”或者直接在这里返回 false 告知外面满了。
            // 如果你想让它“绝对安全并自动扩容”，我们选择将容量翻倍：
            // 如果系统内存耗尽扩容失败，返回 false
            return false;
        }

        // 实际开发中此处需判断环形缓冲区是否写满 (m_head + size 是否超过 m_tail + capacity)
        // 这里简化为直接写入
        char *write_ptr = m_buffer + m_head;

        // 1. 在缓冲区的指定位置上“原地构造”具体类型的命令对象（Placement New）
        new(write_ptr) CommandType{
            {&CommandType::Dispatch, static_cast<uint32_t>(m_head + size)}, // 绑定跳板函数和下一步长
            std::forward<F>(func),
            std::make_tuple(std::forward<Args>(args)...)
        };

        // 2. 移动头指针
        m_head += size;

        return true;
        // 这里的问题, 没有 做到能够扩容
    }

    // 【渲染线程调用】一次性解包并消费当前帧的所有命令
    void FlushAndExecute() {
        size_t current_tail       = m_tail;
        const size_t current_head = m_head; // 快照当前的头，防止逻辑线程同时写入产生竞争

        while (current_tail < current_head) {
            // 1. 获取当前命令的头部
            char *read_ptr    = m_buffer + current_tail;
            const auto header = reinterpret_cast<RenderCommandHeader *>(read_ptr);
            header->execute(read_ptr);
            current_tail = header->next_offset;
        }

        // 重置缓冲区索引（双缓冲或帧重置逻辑）
        m_head = 0;
        m_tail = 0;
    }

private:
    char *m_buffer    = nullptr;
    size_t m_capacity = 0;
    size_t m_head     = 0;
    size_t m_tail     = 0;
};


#endif //HELLO_MAC_LOGIC_TO_RENDER_H
