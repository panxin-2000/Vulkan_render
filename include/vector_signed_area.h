//
// Created by 潘鑫 on 2025/10/1.
//

#ifndef VECTOR_SIGNED_AREA_H
#define VECTOR_SIGNED_AREA_H

#include <vector>
#include <ostream>

#include "point_in_on_out_triangle.h"
#include "bounding_box.h"

class point_2 {
public:
    float x;
    float y;
    using point_type = point_2;

    point_2() {
    }

    point_2(float x1, float y1) {
        x = x1;
        y = y1;
    }

    // point_2 &operator=(const point_2 &R) {
    //     this->x = R.x;
    //     this->y = R.y;
    //     return *this;
    // }

    point_2 operator+(const point_2 &R) const {
        point_2 temp{0, 0};
        temp.x = this->x + R.x;
        temp.y = this->y + R.y;
        return temp;
    }

    // a 的 平方 大于 b 的平方 是返回 true  否则返回 false
    static bool distance_compare(point_2 a, point_2 b) {
        if (a.x * a.x + a.y * a.y > b.x * b.x + b.y * b.y) {
            return true;
        } else {
            return false;
        }
    }

    static point_2 centre_of_a_circle(point_2 a, point_2 b, point_2 c) {
        auto A_1_1 = -2 * (a.x - b.x);
        auto A_1_2 = -2 * (a.y - b.y);
        auto A_2_1 = -2 * (a.x - c.x);
        auto A_2_2 = -2 * (a.y - c.y);
        auto A = A_1_1 * A_2_2 - A_1_2 * A_2_1;
        auto inv_A_1_1 = A_2_2 / A;
        auto inv_A_1_2 = -1 * A_1_2 / A;
        auto inv_A_2_1 = -1 * A_2_1 / A;
        auto inv_A_2_2 = A_1_1 / A;
        auto B_1 = b.x * b.x + b.y * b.y - a.x * a.x - a.y * a.y;
        auto B_2 = c.x * c.x + c.y * c.y - a.x * a.x - a.y * a.y;

        point_2 result = {};
        result.x = inv_A_1_1 * B_1 + inv_A_1_2 * B_2;
        result.y = inv_A_2_1 * B_1 + inv_A_2_2 * B_2;
        return result;
    }

    bool operator==(const point_2 &R);

    friend bool operator<(const point_2 &L, const point_2 &R) {
        if (L.x < R.x) {
            return true;
        } else if (L.x == R.x && L.y < R.y) {
            return true;
        }
        return false;
    }

    friend bool operator==(const point_2 &L, const point_2 &R) {
        if (abs(L.y - R.y) < 0.001 && abs(L.x - R.x) < 0.001) {
            return true;
        }
        return false;
    }

    static point_2 int_max_limit(point_2 &L) {
        L.x = std::numeric_limits<float>::infinity();;
        L.y = std::numeric_limits<float>::infinity();
        return L;
    }

    static point_2 int_min_limit(point_2 &L) {
        L.x = -std::numeric_limits<float>::infinity();;
        L.y = -std::numeric_limits<float>::infinity();
        return L;
    }


    static const point_2 min_two_point(point_2 &L, const point_2 &R) {
        if (R.x < L.x) {
            L.x = R.x;
        }
        if (R.y < L.y) {
            L.y = R.y;
        }
        return L;
    }

    static const point_2 max_two_point(point_2 &L, const point_2 &R) {
        if (R.x > L.x) {
            L.x = R.x;
        }
        if (R.y > L.y) {
            L.y = R.y;
        }
        return L;
    }

    const point_2 operator-(const point_2 &R) const;

    float single_area(const point_2 &R);

    static float single_area(const point_2 &a, const point_2 b, const point_2 c) {
        point_2 ab = b - a;
        point_2 ac = c - a;
        return ab.single_area(ac);
    }


    // 下面一行去掉class之后是能够编译过的，添加之后是编译不过的？
    enum anticlockwise {
        clockwise = 1,
        counterclockwise = 2,
        collinear = 5,
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
    static anticlockwise is_anticlockwise(const point_2 &a, const point_2 &b, const point_2 &c) {
        point_2 ab = b - a;
        point_2 ac = c - a;
        float area = ab.single_area(ac);
        if (abs(area) < 0.00001)
            return anticlockwise::collinear;
        if (area > 0) return anticlockwise::counterclockwise;
        else return anticlockwise::clockwise;
    }
};

class point_3 {
public:
    float x;
    float y;
    float z;
    using point_type = point_3;


    point_3() {
    }

    point_3(float x1, float y1, float z1) {
        x = x1;
        y = y1;
        z = z1;
    }

    point_3 operator+(const point_3 &R) const {
        point_3 temp{0, 0, 0};
        temp.x = this->x + R.x;
        temp.y = this->y + R.y;
        return temp;
    }

    bool operator==(const point_3 &R) {
        if (this->x == R.x && this->y == R.y && this->z == R.z) {
            return true;
        }
        return false;
    }

    friend bool operator<(const point_3 &L, const point_3 &R) {
        if (L.x < R.x) {
            return true;
        }
        return false;
    }

    friend bool operator==(const point_3 &L, const point_3 &R) {
        if (abs(L.y - R.y) < 0.001 && abs(L.x - R.x) < 0.001) {
            return true;
        }
        return false;
    }

    point_3 operator-(const point_3 &R);

    float single_area(const point_3 &R);
};


typedef point_2 triangle_position;

template<typename T>
struct Triangle {
    T a;
    T b;
    T c;

    Triangle(T a_1, T b_1, T c_1) {
        a = a_1;
        b = b_1;
        c = c_1;
    }

    bool operator==(const Triangle &R) {
        if (!(this->a + this->b + this->c == R.a + R.b + R.c)) {
            return false;
        }
        if ((this->a == R.a && this->b == R.b && this->c == R.c) ||
            (this->a == R.b && this->b == R.c && this->c == R.a) ||
            (this->a == R.c && this->b == R.a && this->c == R.b))
            return true;
        else return false;
    }

    bool operator<(const Triangle &R) const {
        auto left = this->a + this->b + this->c;
        auto right = R.a + R.b + R.c;
        if (left < right) {
            return true;
        }
        return false;
    }

    friend std::ostream &operator<<(std::ostream &output,
                                    const Triangle &D) {
        auto right = D.a + D.b + D.c;

        output << " barycenter x : " << right.x << " barycenter y : " << right.y;
        output << " a.x :  " << D.a.x <<
                " a.y :  " << D.a.y <<
                " b.x :  " << D.b.x <<
                " b.y :  " << D.b.y <<
                " c.x :  " << D.c.x <<
                " c.y :  " << D.c.y << std::endl;
        return output;
    }

    friend bool operator==(const Triangle &L, const Triangle &R) {
        if (L == R) {
            return true;
        }
        return false;
    }

    // 有三角形了，需要判断是在三角形里面还是在三角形的边上
    // 有一个固定的算法
    point_in_triangle_type point_position_of_triangle(T point) {
        T a2b = b - a;
        T a2c = c - a;
        float area = a2b.area(a2c);

        float p2a = a - point;
        float alpha = a2c.area(p2a) / area;

        if (abs(area) == 0.000001f) {
            // 三角形退化为一条线了,判断点是否在线上，在的话返回on_edge,不在的话返回为out_triangle
            if ((abs(alpha) == 0.000001f) && (AABB<T>(a, b, c).in_bounding_box(point))) {
                // 判断是否在线上还需要过包围盒，在包围盒内才是在线上
                return on_edge;
            }
            return out_triangle;
        };

        float beta = a2b.area(p2a) / area;
        float gamma = 1.0f - (alpha + beta);

        if (abs(alpha) == 0.000001f || abs(beta) == 0.000001f || abs(gamma) == 0.000001f) {
            return on_edge;
        } else if (alpha < 0.0f || beta < 0.0f || gamma < 0.0f) {
            return out_triangle;
        } else if (alpha > 0.0f || beta > 0.0f || gamma > 0.0f) {
            return in_triangle;
        }
    }


    // 这里如果想要排序，那么也是有点不太一样的需求的
    // 需要比较它们的重心，其实也不是非要比较重心，比较三个值相加也是可以的。
    //
};


bool on_segment_bounding_box(const point_2 &segment_start_point, point_2 &segment_end_point,
                             point_2 &test_point);

struct segment_position {
    point_2 start_point;
    point_2 end_point;

    bool intersection(struct segment_position &R_segment_position);

    bool get_intersection_point(struct segment_position &R_segment_position, point_2 *result);


    static point_2 get_intersection_point(point_2 &start_point, point_2 &end_point, float x) {
        point_2 ab = start_point - end_point;
        point_2 result;
        float a_0 = ab.y / ab.x; // a_0 是 start_point 到 end_point 之间的斜率
        result.x = x;
        result.y = start_point.y + a_0 * (x - start_point.x);
        return result;
    }
};

bool convex_hull_in_order_of_angles(std::vector<point_2> &new_segments);

std::vector<point_2> &calculate_convex_hull(std::vector<point_2> &segments);
#endif //VECTOR_SIGNED_AREA_H
