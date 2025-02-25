//
// Created by 潘鑫 on 2025/2/23.
//
#include <future>
#include <thread>
#include <gtest/gtest.h>


int fibonacci(int n) {
    switch (n) {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 1;
        default:
            return fibonacci(n - 1) + fibonacci(n - 2);
    }
}


int thread_fibonacci(int n) {
    switch (n) {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 1;
        default:
            std::future<int> result_n_1 = std::async(thread_fibonacci, n - 1);
            std::future<int> result_n_2 = std::async(thread_fibonacci, n - 2);
            return (result_n_1.get() + result_n_2.get());
    }
}

/**
 *
 * @param n
 * @param promise  必须加两个引用才能过使用，之后再去看看为什么？
 * @return
 */
int thread_fibonacci_future_promise(int n, std::promise<int> &&promise) {
    switch (n) {
        case 0:
            promise.set_value(0);
            return 0;
        case 1:
        case 2:
            promise.set_value(1);
            return 1;
        default:
            std::promise<int> promise_2;
            std::future<int> future = promise_2.get_future();
            std::thread result_n_1(thread_fibonacci_future_promise, n - 1, std::move(promise_2));
            std::promise<int> promise_1;
            std::future<int> future_1 = promise_1.get_future();
            std::thread result_n_2(thread_fibonacci_future_promise, n - 2, std::move(promise_1));
            int result = future_1.get() + future.get(); // 之前这里总是错，后来发现原来是前两个case没有设置promise的值导致的
            result_n_1.join();
            result_n_2.join();
            promise.set_value(result);
            return result;
    }
}

// 测试递归，顺便测试了async，当需要返回传递的简单参数的时候确实很方便，
// 然后就是各种复杂的返回，比如结构体、类、指针、引用，哪个会出现怎样的结果呢？
// 结构体和类应该会调用复杂的复制构造函数，问题就是这个线程是在什么时候结束的呢？
// 也就是如果结构体存储在栈中能否正确的返回呢？

struct test_async_returns {
    int result;
    int expected;
    int actual;
    int expected_result;
};

test_async_returns test_async_returns_function() {
    test_async_returns tem;
    tem.result = 1;
    tem.expected = 1;
    tem.actual = 1;
    tem.expected_result = 1;
    return tem;
}


TEST(fibonacci, thread) {
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(thread_fibonacci(i), fibonacci(i));
    }
    // for (int i = 0; i < 10; i++) {
    //     EXPECT_EQ(thread_fibonacci_future_promise(i,std::move(nullptr)), fibonacci(i));
    // }   // 想写，但是编译不通过
    for (int i = 0; i < 10; i++) {
        std::promise<int> promise;
        std::future<int> future = promise.get_future();
        int tem = thread_fibonacci_future_promise(i, std::move(promise));
        EXPECT_EQ(tem, fibonacci(i)); //必须要分开写
    } // 想写，但是编译不通过 //本质上是能过递归的，但是差点意思，
    // 运行不过去 // 不是每个分支中都设置的值导致的错误
    std::future<test_async_returns> tr = std::async(test_async_returns_function);
    const test_async_returns &tem = tr.get();
    EXPECT_EQ(1, tem.actual);
    EXPECT_EQ(1, tem.expected);
    EXPECT_EQ(1, tem.expected_result);
    EXPECT_EQ(1, tem.result);
    // 栈中是可以正确返回的
    // 函数能够正确返回，那么async就能够正确返回
}

int callback_add(int a, int b) {
    return a + b;
}

int callback_subtract(int a, int b) {
    return a - b;
}

int callback_mult(int a, int b) {
    return a * b;
}

int callback_division(int a, int b) {
    return a / b;
}

int direct_function_callback(int (*callback_function)(int, int), int l, int r) {
    return callback_function(l, r);
}

TEST(function, callback) {
    struct callback {
        int id;

        int (*add)(int a, int b);
    };


    callback sr = {1, callback_add};
    EXPECT_EQ(5, sr.add(2, 3));
    EXPECT_EQ(5, direct_function_callback(callback_add,2, 3));
    EXPECT_EQ(5, direct_function_callback(callback_subtract,8, 3));
    EXPECT_EQ(6, direct_function_callback(callback_mult,2, 3));
    EXPECT_EQ(5, direct_function_callback(callback_division,15, 3));
}
