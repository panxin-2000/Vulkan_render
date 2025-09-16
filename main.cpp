#include <functional>
#include <iostream>

#include <glm/glm.hpp>

struct Foo {
    Foo(int num) : num_(num) {
    }

    bool print_add(int i) const { std::cout << num_ + i << '\n'; }
    int num_;
};


enum class Color {
    red,
};

void ccv(Color bb) {
}

struct dfg {
    int a;
    int b;
    int c;
};

void test_function(bool b, char a) {
    std::cout << a << '\n';
}

void test_function(char a) {
    std::cout << a << '\n';
}

void load_function(void (*function)(bool b, char a)) {
    function(false, 'a');
}

void load_function(void (*function)(char a)) {
    function('a');
}

int set_value() {
    std::cout << "set_value" << '\n';
    return 42;
}
// static int a = set_value();


class A {

private:
    static int algg;
};

int A::algg = set_value();


int main() {
    float a = -0.0;
    float b = 1 / a;
    float c = -1 / a;
    float cf = 0 / a;
    float cdf =  0 * b;

    int d = 5;

    // const Foo foo(314159);
    // if (5 == 4) {
    // }
    //
    // // store a call to a member function and object
    // using std::placeholders::_1;
    // std::function<bool(int)> f_add_display2 = std::bind(&Foo::print_add, foo, _1);
    // f_add_display2(2);
}
