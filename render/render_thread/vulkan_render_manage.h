//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VULKAN_RENDER_MANAGE_H
#define HELLO_MAC_VULKAN_RENDER_MANAGE_H

#include <oneapi/tbb.h>


#include <thread>
#include <chrono>

#pragma once
#include <thread>
#include <chrono>

#include "record_command.h"

// 判断当前架构以选择正确的 CPU 暂停指令
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#define CPU_PAUSE() _mm_pause()
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)
#define CPU_PAUSE() asm volatile("yield" ::: "memory")
#else
#define CPU_PAUSE() do {} while(0)
#endif

class SpinWait {
private:
    int m_count = 0;

    // 微软经典设定：前 10 次作为自旋观察期
    static constexpr int YIELD_THRESHOLD = 10;

public:
    // 是否已经达到了触发系统切换（Yield/Sleep）的阶段
    bool next_spin_will_yield() const {
        return m_count >= YIELD_THRESHOLD;
    }

    // 重置计数器（每当成功从 tbb::concurrent_queue 拿到数据时调用）
    void reset() {
        m_count = 0;
    }

    // 执行一次渐进式等待
    void spin_once() {
        if (m_count < YIELD_THRESHOLD) {
            // 【第一阶段：自旋步长指数级递增】
            // 模仿硬件总线等待，第1次执行1次pause，第2次执行2次...最高执行到2^4次
            int iterations = 1 << (m_count >> 1); // 巧妙的步长控制
            for (int i = 0; i < iterations; ++i) {
                CPU_PAUSE(); // 释放硬件资源，防止 CPU 烫死
            }
        } else if (m_count < YIELD_THRESHOLD + 10) {
            // 【第二阶段：多次自旋无果，主动让出 CPU 时间片】
            // 允许同优先级的其他物理线程进来干活
            std::this_thread::yield();
        } else {
            // 【第三阶段：彻底陷入睡眠】
            // 逻辑线程太慢了，渲染线程拒绝空转，直接休眠 1ms 控温
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // 防止计数器无限增长引发溢出
        if (m_count < 100) {
            m_count++;
        }
    }
};


class vk_render_queue {
private:
    std::array<oneapi::tbb::concurrent_queue<RenderRingBuffer>, 3> g_render_queue;
    RenderRingBuffer logic_to_render{2 * 1024 * 1024};
    alignas(64) std::atomic<bool> logic_thread_finished = false;
    alignas(64) std::atomic<size_t> logic_write         = 0;
    alignas(64) std::atomic<size_t> render_read         = 1;

public:
    static vk_render_queue &instance() {
        static vk_render_queue *instance = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance = new vk_render_queue();
        });
        return *instance;
    }

    void execute_update_lambda() {
        RenderRingBuffer callback{2 * 1024 * 1024};
        oneapi::tbb::concurrent_queue<RenderRingBuffer> temp;
        if (logic_thread_finished.load() == true) {
            logic_thread_finished.store(false);
            const auto current = render_read % g_render_queue.size();
            while (g_render_queue.at(current).try_pop(callback)) {
                callback.FlushAndExecute();
            }
            g_render_queue.at(current).clear();
        }
    }

    void logic_add_finished() {
        {
            RenderRingBuffer temp{2 * 1024 * 1024};
            std::swap(temp, logic_to_render);
            size_t current = logic_write % g_render_queue.size();
            g_render_queue.at(current).emplace(std::move(temp));
            render_read = current;
            ++logic_write;
            logic_thread_finished.store(true);
        }
    }

    template<typename F, typename... Args>
    void render_update_entt(F &&func, Args &&... args) {
        if (true == logic_to_render.Submit(std::forward<F>(func), std::forward<Args>(args)...)) {
        } else {
            RenderRingBuffer temp{2 * 1024 * 1024};
            std::swap(temp, logic_to_render);
            if (true == logic_to_render.Submit(std::forward<F>(func), std::forward<Args>(args)...)) {
                assert(false && "logic_to_render.Submit");
            }
            g_render_queue.at(logic_write % g_render_queue.size()).emplace(std::move(temp));
        }
    }

    void destroy() {
        RenderRingBuffer callback{2 * 1024 * 1024};
        for (size_t i = 0; i < g_render_queue.size(); ++i) {
            while (g_render_queue.at(i).try_pop(callback)) {
                callback.FlushAndExecute();
            }
            g_render_queue.at(i).clear();
        }
    };

private:
    vk_render_queue() = default;

    ~vk_render_queue() = default;

public:
    vk_render_queue(const vk_render_queue &) = delete;

    vk_render_queue &operator=(const vk_render_queue &) = delete;

    vk_render_queue(vk_render_queue &&) = delete;

    vk_render_queue &operator=(vk_render_queue &&) = delete;
};


#endif //HELLO_MAC_VULKAN_RENDER_MANAGE_H
