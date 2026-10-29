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
    explicit RenderRingBuffer(const size_t capacity) : m_capacity(capacity), next_write_address(0), execute_address(0) {
        // 使用 TBB 线程安全且针对多核心优化的分配器申请一块连续大内存
        const size_t aligned_capacity = (capacity + 255) & ~255; // 向上取整到 256 的倍数
        m_buffer                      = static_cast<char *>(std::aligned_alloc(256, aligned_capacity));
    }

    ~RenderRingBuffer() {
        if (m_buffer != nullptr) {
            free(m_buffer);
        }
    }

    RenderRingBuffer(const RenderRingBuffer &other) = delete;

    RenderRingBuffer &operator=(const RenderRingBuffer &other) = delete;


    // 复制 必须使用 const
    // 移动 必须使用  &&
    RenderRingBuffer(RenderRingBuffer &&other) noexcept
        : m_buffer(other.m_buffer), m_capacity(other.m_capacity), next_write_address(other.next_write_address.load()),
          execute_address(other.execute_address) {
        // 将源对象的指针和状态清空，防止其析构时释放内存
        other.m_buffer           = nullptr;
        other.m_capacity         = 0;
        other.next_write_address = 0;
        other.execute_address    = 0;
    }

    // 3. 实现移动赋值运算符 (Move Assignment Operator)
    RenderRingBuffer &operator=(RenderRingBuffer &&other) noexcept {
        if (this != &other) {
            std::swap(m_buffer, other.m_buffer);
            std::swap(m_capacity, other.m_capacity); {
                const auto temp          = other.next_write_address.load();
                other.next_write_address = next_write_address.load();
                next_write_address.store(temp);
            }
            std::swap(execute_address, other.execute_address);
        }
        return *this;
    }

    friend void swap(RenderRingBuffer &a, RenderRingBuffer &b) noexcept {
        std::swap(a.m_buffer, b.m_buffer);
        std::swap(a.m_capacity, b.m_capacity); {
            const auto temp      = b.next_write_address.load();
            b.next_write_address = a.next_write_address.load();
            a.next_write_address.store(temp);
        }
        std::swap(a.execute_address, b.execute_address);
    }


    // 最初设计时,逻辑线程和 渲染线程就是分离的 ,不会同时读和写同一个, 因为是分离, 需要全部写完之后,再提交渲染线程,所以是暂时能用的
    // 之后想要自行扩充内存, 那么是必须要执行加锁的申请更多的内存的
    // 【逻辑线程调用】向缓冲区提交任意函数和参数
    template<typename F, typename... Args>
    bool Submit(F &&func, Args &&... args) {
        using CommandType            = ConcreteRenderCommand<std::decay_t<F>, std::decay_t<Args>...>;
        const size_t size            = ALIGN_16(sizeof(CommandType)); // 我需要的字节数
        size_t current_write_address = next_write_address.load();     // 当前能够直接写的地址, 需要16字节对齐
        while (true) {
            if (current_write_address + size > m_capacity) {
                return false;
            }
            if (next_write_address.compare_exchange_strong(current_write_address, current_write_address + size,
                                                           std::memory_order_seq_cst,
                                                           std::memory_order_relaxed)) {
                // next_write_address 这里还应该是 16 字节对齐的
                char *write_ptr = m_buffer + current_write_address;
                new(write_ptr) CommandType{
                    {&CommandType::Dispatch, static_cast<uint32_t>(current_write_address + size)},
                    std::forward<F>(func),
                    std::make_tuple(std::forward<Args>(args)...)
                };
                return true;
            }
        }
    }

    // 【渲染线程调用】一次性解包并消费当前帧的所有命令
    void FlushAndExecute() {
        // 这里的特殊之处 在于 渲染线程 执行时, 逻辑线程已经确定全部写完了 , 才swap 到渲染线程中
        const size_t current_head = next_write_address; // 快照当前的头，防止逻辑线程同时写入产生竞争

        while (execute_address < current_head) {
            // 1. 获取当前命令的头部
            char *read_ptr    = m_buffer + execute_address;
            const auto header = reinterpret_cast<RenderCommandHeader *>(read_ptr);
            header->execute(read_ptr);
            execute_address = header->next_offset;
        }

        // 重置缓冲区索引（双缓冲或帧重置逻辑）
        next_write_address = 0;
        execute_address    = 0;
    }

private:
    char *m_buffer                         = nullptr;
    size_t m_capacity                      = 0;
    std::atomic<size_t> next_write_address = 0;
    size_t execute_address                 = 0;
};


#endif //HELLO_MAC_LOGIC_TO_RENDER_H
