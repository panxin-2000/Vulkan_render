//
// Created by 潘鑫 on 2025/12/4.
//

#ifndef HELLO_MAC_POINT_2_H
#define HELLO_MAC_POINT_2_H
#include <ostream>

class Point_2 {
public:
    float x;
    float y;
    using point_type = Point_2;

    Point_2() = default;

    Point_2(float x1, float y1) : x(x1), y(y1) {
    }

    friend std::ostream &operator<<(std::ostream &output,
                                    const Point_2 &P) {
        output << "(x,y):(" << P.x << "," << P.y << ")";
        return output;
    }

    // point_2 &operator=(const point_2 &R) {
    //     this->x = R.x;
    //     this->y = R.y;
    //     return *this;
    // }


    // a 的 平方 大于 b 的平方 是返回 true  否则返回 false
    static bool distance_compare(Point_2 a, Point_2 b) {
        if (a.x * a.x + a.y * a.y > b.x * b.x + b.y * b.y) {
            return true;
        } else {
            return false;
        }
    }

    static Point_2 centre_of_a_circle(Point_2 a, Point_2 b, Point_2 c) {
        auto A_1_1     = -2 * (a.x - b.x);
        auto A_1_2     = -2 * (a.y - b.y);
        auto A_2_1     = -2 * (a.x - c.x);
        auto A_2_2     = -2 * (a.y - c.y);
        auto A         = A_1_1 * A_2_2 - A_1_2 * A_2_1;
        auto inv_A_1_1 = A_2_2 / A;
        auto inv_A_1_2 = -1 * A_1_2 / A;
        auto inv_A_2_1 = -1 * A_2_1 / A;
        auto inv_A_2_2 = A_1_1 / A;
        auto B_1       = b.x * b.x + b.y * b.y - a.x * a.x - a.y * a.y;
        auto B_2       = c.x * c.x + c.y * c.y - a.x * a.x - a.y * a.y;

        const Point_2 result{inv_A_1_1 * B_1 + inv_A_1_2 * B_2, inv_A_2_1 * B_1 + inv_A_2_2 * B_2};
        return result;
    }


    friend bool operator<(const Point_2 &L, const Point_2 &R) {
        if (L.x < R.x) {
            return true;
        } else if (L.x == R.x && L.y < R.y) {
            return true;
        }
        return false;
    }

    friend bool operator<=(const Point_2 &L, const Point_2 &R) {
        if (L.x <= R.x && L.y <= R.y) {
            return true;
        }
        return false;
    }

    friend bool operator==(const Point_2 &L, const Point_2 &R) {
        if (abs(L.y - R.y) < 0.000001 && abs(L.x - R.x) < 0.000001) {
            return true;
        }
        return false;
    }

    static Point_2 init_max_limit() {
        return {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
    }

    static Point_2 init_min_limit() {
        return {-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()};
    }


    static Point_2 min_two_point(const Point_2 &L, const Point_2 &R) {
        return {((R.x < L.x) ? R.x : L.x), ((R.y < L.y) ? R.y : L.y)};
    }

    static Point_2 max_two_point(Point_2 &L, const Point_2 &R) {
        return {((R.x > L.x) ? R.x : L.x), ((R.y > L.y) ? R.y : L.y)};
    }

    Point_2 operator+(const Point_2 &R) const {
        Point_2 temp{0, 0};
        temp.x = this->x + R.x;
        temp.y = this->y + R.y;
        return temp;
    }

    Point_2 operator/(const float number) const {
        Point_2 temp{0, 0};
        temp.x = this->x / number;
        temp.y = this->y / number;
        return temp;
    }

    Point_2 operator/(const Point_2 number) const {
        Point_2 temp{0, 0};
        temp.x = this->x / number.x;
        temp.y = this->y / number.y;
        return temp;
    }

    Point_2 operator*(const float number) const {
        Point_2 temp{0, 0};
        temp.x = this->x * number;
        temp.y = this->y * number;
        return temp;
    }

    Point_2 operator*(const Point_2 number) const {
        Point_2 temp{0, 0};
        temp.x = this->x * number.x;
        temp.y = this->y * number.y;
        return temp;
    }

    Point_2 operator-(const Point_2 &R) const {
        Point_2 temp{0, 0};
        temp.x = this->x - R.x;
        temp.y = this->y - R.y;
        return temp;
    }

    friend Point_2 abs(const Point_2 &R) {
        const Point_2 temp{std::abs(R.x), std::abs(R.y)};
        return temp;
    }


    /**
     * 可以用来判断顺时针还是逆时针，第二个相对于第一个逆时针为正，顺时针为负
     * @param R
     * @return
     */
    static float cross_product(const Point_2 &L, const Point_2 &R) {
        return L.x * R.y - L.y * R.x;
    }

    static float single_area(const Point_2 &a, const Point_2 b, const Point_2 c) {
        const Point_2 ab = b - a;
        const Point_2 ac = c - a;
        return cross_product(ab, ac);
    }


    // 下面一行去掉class之后是能够编译过的，添加之后是编译不过的？
    enum anticlockwise {
        clockwise        = 1,
        counterclockwise = 2,
        collinear        = 4,
        // 下面的四个是方便组合的
        clockwise_and_counterclockwise  = 3,
        collinear_and_clockwise         = 5,
        collinear_and_counterclockwise  = 6,
        collinear_and_clock_and_counter = 7,
    };

    friend anticlockwise operator&(anticlockwise &left, anticlockwise &right) {
        return static_cast<anticlockwise>(static_cast<int>(left) & static_cast<int>(right));
    }


    /**
     * 按照顺序输入三个点，如果是逆时针的话，那么返回 true,否则返回 false
     * @param a
     * @param b
     * @param c
     * @return
     */
    static anticlockwise is_anticlockwise(const Point_2 &a, const Point_2 &b, const Point_2 &c) {
        Point_2 ab = b - a;
        Point_2 ac = c - a;
        float area = cross_product(ab, ac);
        if (abs(area) < 0.00001)
            return anticlockwise::collinear;
        if (area > 0) return anticlockwise::counterclockwise;
        else return anticlockwise::clockwise;
    }
};

inline float dot(const Point_2 &A, const Point_2 &b) {
    return A.x * b.x + A.y * b.y;
}

inline float cross_product(const Point_2 &A, const Point_2 &b) {
    return Point_2::cross_product(A, b);
}

inline Point_2 clamp(const Point_2 input, const Point_2 min, const Point_2 max) {
    auto result_x = std::clamp(input.x, min.x, max.x);
    auto result_y = std::clamp(input.y, min.y, max.y);
    return {result_x, result_y};
}

#endif //HELLO_MAC_POINT_2_H
