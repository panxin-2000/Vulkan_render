//
// Created by 潘鑫 on 2025/2/21.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <gtest/gtest.h>


std::mutex mtx;
int shared_resource = 0;

void increment() {
    for (int i = 0; i < 10000; i++) {
        mtx.lock();
        ++shared_resource;
        mtx.unlock();
    }
}

void increment_not_lock() {
    for (int i = 0; i < 10000; i++) {
        ++shared_resource;
    }
}

/**
 * 一种自动管理 std::mutex 锁的封装器，使用 RAII 风格，确保在作用域结束时自动释放锁。
 */
void increment_guard() {
    for (int i = 0; i < 10000; i++) {
        std::lock_guard<std::mutex> lock(mtx);
        ++shared_resource;
    }
}

/**
 * 提供比 std::lock_guard 更灵活的锁管理，可以手动释放和重新获得锁，还支持定时锁定。
 */
void increment_unique() {
    for (int i = 0; i < 10000; i++) {
        std::unique_lock<std::mutex> lock(mtx);
        ++shared_resource;
        lock.unlock();
    }
}

void increment_unique_auto_unlock() {
    for (int i = 0; i < 10000; i++) {
        std::unique_lock<std::mutex> lock(mtx);
        ++shared_resource;
        lock.unlock();
    }
}


TEST(test_mutex, just_mutex) {
    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();

    EXPECT_EQ(shared_resource, 20000);
    shared_resource = 0;

    std::thread t3(increment_not_lock);
    std::thread t4(increment_not_lock);

    t3.join();
    t4.join();
    EXPECT_LE(shared_resource, 20000);
    // Expected: (shared_resource) < (20000), actual: 20000 vs 20000
    // 上面着一行偶尔会出现问题

    shared_resource = 0;
    std::thread t5(increment_guard);
    std::thread t6(increment_guard);

    t5.join();
    t6.join();
    EXPECT_EQ(shared_resource, 20000);

    shared_resource = 0;
    std::thread t7(increment_unique);
    std::thread t8(increment_unique);
    t7.join();
    t8.join();
    EXPECT_EQ(shared_resource, 20000);

    shared_resource = 0;
    std::thread t9(increment_unique_auto_unlock);
    std::thread t10(increment_unique_auto_unlock);
    t9.join();
    t10.join();
    EXPECT_EQ(shared_resource, 20000);

}
