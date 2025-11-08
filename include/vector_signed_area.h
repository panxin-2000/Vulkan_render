//
// Created by 潘鑫 on 2025/10/1.
//

#ifndef VECTOR_SIGNED_AREA_H
#define VECTOR_SIGNED_AREA_H

#include <vector>
#include <ostream>

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

    static  float single_area(const point_2 &a, const point_2 b, const point_2 c) {
        point_2 ab = b - a;
        point_2 ac = c - a;
        return ab.single_area(ac);
    }

    static bool is_anticlockwise(const point_2 &a, point_2 b, point_2 c);
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
struct triangle {
    T a;
    T b;
    T c;

    triangle(T a_1, T b_1, T c_1) {
        a = a_1;
        b = b_1;
        c = c_1;
    }

    bool operator==(const triangle &R) {
        if (!(this->a + this->b + this->c == R.a + R.b + R.c)) {
            return false;
        }
        if ((this->a == R.a && this->b == R.b && this->c == R.c) ||
            (this->a == R.b && this->b == R.c && this->c == R.a) ||
            (this->a == R.c && this->b == R.a && this->c == R.b))
            return true;
        else return false;
    }

    bool operator<(const triangle &R) const {
        auto left = this->a + this->b + this->c;
        auto right = R.a + R.b + R.c;
        if (left < right) {
            return true;
        }
        return false;
    }

    friend std::ostream &operator<<(std::ostream &output,
                                    const triangle &D) {
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

    friend bool operator==(const triangle &L, const triangle &R) {
        if (L == R) {
            return true;
        }
        return false;
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
