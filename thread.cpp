//
// Created by 潘鑫 on 2025/2/21.
//
#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>

std::mutex m;
std::condition_variable cv;
//  std::condition_variable  状态变量，怎么在线程中传递，
std::string data;
bool ready = false;
bool processed = false;

void worker_thread() {
    // wait until main() sends data

    // 应该是等到通知后再去上锁的，而不应该是先上锁，然后在等待
    std::unique_lock lk(m);
    cv.wait(lk, [] { return ready; });

    // after the wait ,we own the lock
    std::cout << "worker thread is processing data \n";
    data += " after processing";

    // send data back to main()
    processed = true;
    std::cout  << "worker thread signals data processing completed\n";

    //manual unlocking is done before notifying ,to avoid waking up
    // the waiting thread only to block again (see notify_one for details)

    lk.unlock();
    cv.notify_one();

}


void foo() {
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::thread::id this_id = std::this_thread::get_id();
    std::cout << "thread " << this_id << std::endl;
    std::cout << "thread " << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void bar() {
    // simulate expensive operation
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

std::mutex mtx;//
int shared_resource = 0;

int test_thread() {
    std::cout << "start first helper .. \n";
    std::thread t1(&foo);
    std::cout << "start second helper .. \n";
    std::thread t2(&bar);

    std::cout << "waiting for helpers to finish \n";
    t1.join();
    t2.join();
    std::cout << "done\n";

    std::thread worker(worker_thread);

    data = "example data";
    {
        std::lock_guard<std::mutex> lk(m);
        ready = true;
        std::cout << "main() siganls data ready for processing \n";
    }
    cv.notify_one();
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [] { return processed; });
    }
    std::cout << "back in main() data = " << data << std::endl;
    worker.join();
    // 互斥量与lock的关系？为什么一定要这么写？
    // lock是互斥量的一个包装其，封装了一些操作
}
