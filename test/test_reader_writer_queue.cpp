//
// Created by 潘鑫 on 2026/5/18.
//
#include <thread>
#include "gtest/gtest.h"
#include <readerwriterqueue.h>


// 顺时针
TEST(no_locked, fsd) {
    moodycamel::BlockingReaderWriterQueue<int> queue;

    std::thread reader([&]() {
        int item;
#if 1
        for (int i = 0; i != 100; ++i) {
            // Fully-blocking:
            queue.wait_dequeue(item);
            EXPECT_EQ(item, i);
        }
#else
        for (int i = 0; i != 100;) {
            // Blocking with timeout
            if (queue.wait_dequeue_timed(item, std::chrono::milliseconds(5)))
                ++i;
        }
#endif
    });
    std::thread writer([&]() {
        for (int i = 0; i != 100; ++i) {
            queue.enqueue(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    writer.join();
    reader.join();


    assert(queue.size_approx() == 0);
}
