//
// Created by 潘鑫 on 2025/10/1.
//

#ifndef VECTOR_SIGNED_AREA_H
#define VECTOR_SIGNED_AREA_H

#include <vector>
#include <ostream>
#include "base_element/point_2.h"
#include "point_in_on_out_triangle.h"
#include "base_element/AABB_bounding_box.h"


typedef Point_2 triangle_position;

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
        float area = a2b.single_area(a2c);

        auto a2p = point - a;
        float alpha = a2p.single_area(a2c) / area;

        if (abs(area) == 0.000001f) {
            // 三角形退化为一条线了,判断点是否在线上，在的话返回on_edge,不在的话返回为out_triangle
            if (abs(alpha) == 0.000001f) {
                if (intersect(AABB_centroid<T>{a, b, c}, point))return on_edge;
                // 判断是否在线上还需要过包围盒，在包围盒内才是在线上
            }
            return out_triangle;
        };

        float beta = a2b.single_area(a2p) / area;
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





bool convex_hull_in_order_of_angles(std::vector<Point_2> &new_segments);

std::vector<Point_2> &calculate_convex_hull(std::vector<Point_2> &segments);
#endif //VECTOR_SIGNED_AREA_H
