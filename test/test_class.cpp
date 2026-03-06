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
struct binary_Tree_Node {
public:
    binary_Tree_Node() {
    }

    ~binary_Tree_Node(void) {
    }

    binary_Tree_Node *Left_child; // 因为是64位的，所以指针占据了8个字节
    binary_Tree_Node *Right_child;
    binary_Tree_Node *Father_Node;
    T data;
    int color_tag;
};


struct tem {
    int number_a;
};

class class_c : public binary_Tree_Node<tem> {
public:
    int number_b;
};

TEST(class_access2, class_access) {
    class_c c{};
    int a = sizeof(class_a);
    int b = sizeof(class_b);
    int f = sizeof(class_c);
    int r = sizeof(binary_Tree_Node<tem>);
    ASSERT_EQ(0, (int64_t)&c - (int64_t)&c.Left_child);
    ASSERT_EQ(-8, (int64_t)&c - (int64_t)&c.Right_child);
    ASSERT_EQ(-16, (int64_t)&c - (int64_t)&c.Father_Node);
    ASSERT_EQ(-24, (int64_t)&c - (int64_t)&c.data.number_a);
    // ASSERT_EQ(-28, (int64_t)&c - (int64_t)&c.number_b);// 不添加color_flag是是对的，添加之后就不对了
    ASSERT_EQ(-32, (int64_t)&c - (int64_t)&c.number_b);
    // 一直没有在意过指针对内存的影响

    c.data.number_a = 20;
    c.Left_child    = nullptr;
    c.Right_child   = nullptr;
    // c.Father_Node = nullptr;
    ASSERT_EQ(c.data.number_a, 20);
    // 想要可以展开一个结构体，那么可以使用class
    // 其实可以这样理解，不管你是偏移一次还是偏移两次，
    // 真实的到汇编的时候应该都是计算一个偏移
}

class random_func {
public:
    int value;
    std::string name_;

    random_func(int a, std::string name) {
        value = a;
        name_ = name;
        std::cout << name_ << " 构造函数 " << value << std::endl;
    }

    ~random_func() {
        if (value == 0) {
            std::cout << name_ << " 析构函数 但是不需要释放资源 " << value << std::endl;
        } else {
            std::cout << name_ << " 析构函数 " << value << std::endl;
        }
    };

    // 复制构造函数 和 复制赋值运算符 需要执行 两遍 析构函数

    // 移动构造函数 和 移动赋值运算符 只会执行 一遍 析构函数
    // 被移动的内容会执行一个 清理 ，不清理

    random_func(const random_func &other)
        : value(other.value) {
        std::cout << name_ << " 复制构造函数" << other.name_ << value << std::endl;
    }

    random_func(random_func &&other) noexcept
        : value(other.value) {
        std::cout << other.name_ << " 需要清理的句柄或其他 " << value << std::endl;
        other.value = 0;
        std::cout << name_ << " 移动构造函数 清理完成后的值" << other.name_ << value << std::endl;
    }

    random_func &operator=(const random_func &other) {
        if (this == &other)
            return *this;
        value = other.value;
        std::cout << name_ << " 复制赋值运算符" << other.name_ << value << std::endl;
        return *this;
    }

    random_func &operator=(random_func &&other) noexcept {
        if (this == &other)
            return *this;
        value = other.value;
        std::cout << other.name_ << " 需要清理的句柄或其他 " << value << std::endl;
        other.value = 0;
        std::cout << name_ << " 移动赋值运算符 清理完成后的值 " << other.name_ << value << std::endl;

        return *this;
    }
};

TEST(move_and_copy, construction_and_destruction) {
    random_func a = {10, "a"};
    random_func b = {20, "b"};

    random_func e = {10, "e"};
    random_func f = {20, "f"};
    EXPECT_EQ(a.value, 10);
    EXPECT_EQ(b.value, 20);
    b = (a);
    EXPECT_EQ(a.value, 10);
    EXPECT_EQ(b.value, 10);

    b = std::move(a);

    EXPECT_EQ(a.value, 0);
    EXPECT_EQ(b.value, 10);

    random_func c{e};
    EXPECT_EQ(c.value, 10);
    EXPECT_EQ(e.value, 10);

    random_func d{std::move(f)};
    EXPECT_EQ(d.value, 20);
    EXPECT_EQ(f.value, 0);
}
