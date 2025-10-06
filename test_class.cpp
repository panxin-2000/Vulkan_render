//
// Created by 潘鑫 on 2025/9/24.
//
#include  <gtest/gtest.h>

class class_a {
public:
    int number_a;
};


class class_b : public class_a {
public:
    int number_b;
};

TEST(class_access, class_access) {
    class_b b{};
    b.number_a = 10;
    ASSERT_EQ(b.number_a, 10);
}

template<class T>
struct balance_Tree_Node {
public:
    balance_Tree_Node() {
    }

    ~balance_Tree_Node(void) {
    }

    balance_Tree_Node *Left_child; // 因为是64位的，所以指针占据了8个字节
    balance_Tree_Node *Right_child;
    balance_Tree_Node *Father_Node;
    T data;
    int color_tag;
};


struct tem {
    int number_a;
};

class class_c : public balance_Tree_Node<tem> {
public:
    int number_b;
};

TEST(class_access2, class_access) {
    class_c c{};
    int a = sizeof(class_a);
    int b = sizeof(class_b);
    int f = sizeof(class_c);
    int r = sizeof(balance_Tree_Node<tem>);
    ASSERT_EQ(0, (int64_t)&c - (int64_t)&c.Left_child);
    ASSERT_EQ(-8, (int64_t)&c - (int64_t)&c.Right_child);
    ASSERT_EQ(-16, (int64_t)&c - (int64_t)&c.Father_Node);
    ASSERT_EQ(-24, (int64_t)&c - (int64_t)&c.data.number_a);
    // ASSERT_EQ(-28, (int64_t)&c - (int64_t)&c.number_b);// 不添加color_flag是是对的，添加之后就不对了
    ASSERT_EQ(-32, (int64_t)&c - (int64_t)&c.number_b);
    // 一直没有在意过指针对内存的影响

    c.data.number_a = 20;
    c.Left_child = nullptr;
    c.Right_child = nullptr;
    // c.Father_Node = nullptr;
    ASSERT_EQ(c.data.number_a, 20);
    // 想要可以展开一个结构体，那么可以使用class
    // 其实可以这样理解，不管你是偏移一次还是偏移两次，
    // 真实的到汇编的时候应该都是计算一个偏移
}
