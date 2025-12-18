//
// Created by 潘鑫 on 2025/4/24.
//
#include "sample3.h"
#include "gtest/gtest.h"

namespace {
    class QueueTestSmpl3 : public ::testing::Test {
    protected:
        void SetUp() override {
            q0_.Enqueue(1);
            q1_.Enqueue(2);
            q2_.Enqueue(3);
        }


        static int Double(int n) { return n * 2; }

        void MapTester(const Queue<int> *q) {
            const Queue<int> *const new_q = q->Map(Double);

            ASSERT_EQ(q->size(), new_q->size());

            for (const QueueNode<int> *n1 = q->Head(), *n2 = new_q->Head();
                 n1 != nullptr; n1 = n1->next(), n2 = n2->next()) {
                EXPECT_EQ(2 * n1->element(), n2->element());
            }

            delete new_q;
        }

        Queue<int> q0_;
        Queue<int> q1_;
        Queue<int> q2_;
    };



    // Tests the default c'tor.
    TEST_F(QueueTestSmpl3, DefaultConstructor) {
        // You can access data in the test fixture here.
        EXPECT_EQ(0u, q0_.size());
    }

    // Tests Dequeue().
    TEST_F(QueueTestSmpl3, Dequeue) {
        int* n = q0_.Dequeue();
        EXPECT_TRUE(n == nullptr);

        n = q1_.Dequeue();
        ASSERT_TRUE(n != nullptr);
        EXPECT_EQ(1, *n);
        EXPECT_EQ(0u, q1_.size());
        delete n;

        n = q2_.Dequeue();
        ASSERT_TRUE(n != nullptr);
        EXPECT_EQ(2, *n);
        EXPECT_EQ(1u, q2_.size());
        delete n;
    }

    // Tests the Queue::Map() function.
    TEST_F(QueueTestSmpl3, Map) {
        MapTester(&q0_);
        MapTester(&q1_);
        MapTester(&q2_);
    }


}
