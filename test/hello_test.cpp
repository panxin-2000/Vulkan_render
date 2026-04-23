//
// Created by 潘鑫 on 2025/2/5.
//
//
#include <random>
#include <gtest/gtest.h>


#define MIN(x, y)({ typeof(x)_x = (x);typeof(y)_y = (y);(void)(&_x==&_y);_x<_y?_x:_y;})
#define MAX(x, y)({ typeof(x)_x = (x);typeof(y)_y = (y);(void)(&_x==&_y);_x>_y?_x:_y;})

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    //    EXPECT_STRNE("hello", "hello");
    //    Expected: ("hello") != ("hello"), actual: "hello" vs "hello"

    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}


TEST(min_of_two_number, min) { {
        EXPECT_EQ(MIN(9, 4), 4);
    } {
        EXPECT_EQ(MAX(9, 4), 9);
    }
}

TEST(min_of_two_number, max) {
    //    EXPECT_EQ(MIN(9, 4), 4);
    EXPECT_EQ(MAX(9, 4), 9);
}


void do_not_optimize(int val) {
    static volatile int sink = 0;
    sink                     = val;
}

// 结果不同，
// | 步长: 4 Bytes | 平均单次访问耗时: 2.14763 ns
// | 步长: 64 Bytes | 平均单次访问耗时: 2.28966 ns
// | 步长: 128 Bytes | 平均单次访问耗时: 6.30999 ns
// | 步长: 256 Bytes | 平均单次访问耗时: 6.73759 ns
// | 步长: 512 Bytes | 平均单次访问耗时: 5.18135 ns
// | 步长: 1024 Bytes | 平均单次访问耗时: 3.60168 ns
// | 步长: 2048 Bytes | 平均单次访问耗时: 4.05453 ns
// | 步长: 4096 Bytes | 平均单次访问耗时: 4.47418 ns
void run_test(const std::string &label, size_t size, size_t stride) {
    std::vector<int> data(size, 1);
    size_t count = 0;

    auto start = std::chrono::high_resolution_clock::now();

    // 循环多次以获得稳定数据
    for (int r = 0; r < 100; ++r) {
        for (size_t i = 0; i < size; i += stride) {
            count += data[i];
        }
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    do_not_optimize(count);

    std::cout << label << " | 步长: " << stride * sizeof(int) << " Bytes | "
            << "平均单次访问耗时: " << (double) duration / (100 * (size / stride)) << " ns" << std::endl;
}

// 完全破坏了随机
//  | 步长: 4 Bytes | 平均单次访问耗时: 38.984 ns
//  | 步长: 64 Bytes | 平均单次访问耗时: 37.3055 ns
//  | 步长: 128 Bytes | 平均单次访问耗时: 38.4734 ns
//  | 步长: 256 Bytes | 平均单次访问耗时: 36.3757 ns
//  | 步长: 512 Bytes | 平均单次访问耗时: 36.9555 ns
//  | 步长: 1024 Bytes | 平均单次访问耗时: 35.7475 ns
//  | 步长: 2048 Bytes | 平均单次访问耗时: 38.9773 ns
//  | 步长: 4096 Bytes | 平均单次访问耗时: 36.6771 ns
void run_test_random(const std::string &label, size_t size, size_t stride) {
    std::vector<int> data(size, 1);
    size_t count = 0;

    std::random_device rd;
    std::mt19937 g(rd());
    std::vector<size_t> indices(size / stride);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), g);
    const auto start = std::chrono::high_resolution_clock::now();

    // 循环多次以获得稳定数据
    for (const auto idx: indices) {
        do_not_optimize(data[idx * stride]); // 此时预取器彻底废掉
    }

    const auto end      = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();


    std::cout << label << " | 步长: " << stride * sizeof(int) << " Bytes | "
            << "平均单次访问耗时: " << (double) duration / indices.size() << " ns" << std::endl;
}


TEST(test_run_time, max) {
    // 执行一条纯计算指令的速度通常远小于 1 ns
    // 如果你的 CPU 频率是 3.0 GHz，那么时钟周期就是 0.33 ns。
    // 1 - 10 ns 数据已经被预取到了 L1 或 L2 缓存
    // 现代 CPU 绝大部分时间都在“等数据”，而不是在“算数据”
    // GPU 的内存延迟显著高于 CPU，通常在 数百个时钟周期（约 100ns 到 800ns 甚至更高） 之间
    run_test("Possible Cache Miss    ", 16 * 1024 * 1024, 1);
    run_test("Possible Cache Miss    ", 16 * 1024 * 1024, 16);
    run_test("Possible Cache Miss    ", 16 * 1024 * 1024, 32);
    run_test("Possible Cache Miss    ", 16 * 1024 * 1024, 64);
    run_test("Possible Cache Miss    ", 16 * 1024 * 1024, 128);
    run_test("Possible Cache Miss    ", 16 * 1024 * 1024, 256);
    run_test("Possible Cache Miss    ", 16 * 1024 * 1024, 512);
    run_test("Possible Cache Miss    ", 16 * 1024 * 1024, 1024);
}

struct ss {
    int a;
    int b;
    float c;
    double d;
    bool operator==(const ss &r) const { return a == r.a && b == r.b && c == r.c && d == r.d; };
};

TEST(ss, struct_eqeue) {
    struct ss s  = {1, 2, 3.0, 4.6};
    struct ss sc = {1, 2, 3.0, 4.6};
    EXPECT_EQ(s, sc); // 其实还是调用了C++中的代码，是不是不知方便显示结果或者组合测试示例？
}
