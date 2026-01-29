//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_EVENT_QUEUE_MANGE_H
#define LEARN_OPENGL_EVENT_QUEUE_MANGE_H

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <stdexcept>
#include <utility>

// 通用线程安全队列（容量通过模板参数固定，0表示无限制）
template<typename T, size_t MAX_SIZE = 255>
class queue_thread_safe {
public:
    // 类型别名
    using ValueType = T;

private:
    std::queue<ValueType> queue_;          // 底层队列
    mutable std::mutex mtx_;               // 互斥锁（mutable支持const方法加锁）
    std::condition_variable cv_not_empty_; // 队列非空条件变量
    std::condition_variable cv_not_full_;  // 队列非满条件变量（仅MAX_SIZE>0时生效）
    std::atomic<bool> is_closed_;          // 队列关闭状态
public:
    static constexpr size_t MaxSize = MAX_SIZE; // 编译期可见的最大容量

    // 构造函数：无需传入容量（模板参数已固定）
    queue_thread_safe() : is_closed_(false) {
    }

    // 禁用拷贝/移动（锁和条件变量不可拷贝）
    queue_thread_safe(const queue_thread_safe &) = delete;

    queue_thread_safe &operator=(const queue_thread_safe &) = delete;

    queue_thread_safe(queue_thread_safe &&) = delete;

    queue_thread_safe &operator=(queue_thread_safe &&) = delete;

    // -------------------------- 核心接口 --------------------------
    /**
     * 入队（阻塞版）：队列满时阻塞，直到有空间或队列关闭
     * @param value 待入队元素（移动语义）
     * @throw runtime_error 队列已关闭时抛出异常
     */
    void push(ValueType value) {
        std::unique_lock<std::mutex> lock(mtx_);

        // 队列已关闭，拒绝入队
        if (is_closed_) {
            throw std::runtime_error("Push to closed ThreadSafeQueue");
        }

        // 若模板参数MAX_SIZE>0，阻塞等待队列有空间
        if constexpr (MAX_SIZE > 0) {
            // 编译期分支（无运行时开销）
            cv_not_full_.wait(lock, [this]() {
                return is_closed_ || queue_.size() < MAX_SIZE;
            });
        }

        // 再次检查队列状态
        if (is_closed_) {
            throw std::runtime_error("Push to closed ThreadSafeQueue");
        }

        // 入队（移动语义）
        queue_.emplace(std::move(value));
        cv_not_empty_.notify_one(); // 唤醒出队线程
    }

    /**
     * 出队（阻塞版）：队列为空时阻塞，直到有元素或队列关闭
     * @return 可选类型：有元素返回T，队空且关闭返回std::nullopt
     */
    std::optional<ValueType> pop() {
        std::unique_lock<std::mutex> lock(mtx_);

        // 阻塞等待：队列非空 或 队列关闭
        cv_not_empty_.wait(lock, [this]() {
            return is_closed_ || !queue_.empty();
        });

        // 队列关闭且无元素，返回空
        if (is_closed_ && queue_.empty()) {
            return std::nullopt;
        }

        // 出队（移动元素）
        ValueType value = std::move(queue_.front());
        queue_.pop();

        // 若模板参数MAX_SIZE>0，唤醒入队线程（队列有空间了）
        if constexpr (MAX_SIZE > 0) {
            cv_not_full_.notify_one();
        }

        return value;
    }

    /**
     * 非阻塞出队：立即返回，不阻塞
     */
    std::optional<ValueType> try_pop() {
        std::unique_lock<std::mutex> lock(mtx_);

        if (queue_.empty()) {
            return std::nullopt;
        }

        ValueType value = std::move(queue_.front());
        queue_.pop();

        if constexpr (MAX_SIZE > 0) {
            cv_not_full_.notify_one();
        }

        return value;
    }

    // -------------------------- 辅助接口 --------------------------
    // 关闭队列
    void close() {
        std::unique_lock<std::mutex> lock(mtx_);
        is_closed_ = true;
        cv_not_empty_.notify_all();
        cv_not_full_.notify_all();
    }

    // 检查队列是否关闭
    bool is_closed() const noexcept {
        return is_closed_;
    }

    // 获取当前队列大小
    size_t size() const {
        std::unique_lock<std::mutex> lock(mtx_);
        return queue_.size();
    }

    // 检查队列是否为空
    bool empty() const {
        std::unique_lock<std::mutex> lock(mtx_);
        return queue_.empty();
    }

    // 检查队列是否已满（仅当MAX_SIZE>0时可用）
    bool is_full() const {
        static_assert(MAX_SIZE != 0, "error MAX_SIZE == 0");
        std::unique_lock<std::mutex> lock(mtx_);
        return queue_.size() >= MAX_SIZE;
    }
};

// 常用事件队列类型别名（简化使用）
// using ComboKeyEventQueue = EventQueueManager<ComboKeyEvent>;
// using MouseClickEventQueue = EventQueueManager<MouseClickEvent>;
// using WindowResizeEventQueue = EventQueueManager<WindowResizeEvent>;
#endif //LEARN_OPENGL_EVENT_QUEUE_MANGE_H
