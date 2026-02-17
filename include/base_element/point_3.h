//
// Created by 潘鑫 on 2025/12/4.
//

#ifndef HELLO_MAC_POINT_3_H
#define HELLO_MAC_POINT_3_H
#include "point_2.h"

class Point_3 {
public:
    float x;
    float y;
    float z;
    using point_type = Point_3;


    Point_3() {
    }

    Point_3(Point_2 xy) : x(xy.x), y(xy.y), z(0) {
    }

    Point_3(float x1, float y1, float z1) : x(x1), y(y1), z(z1) {
    }

    Point_3 operator+(const Point_3 &R) const {
        Point_3 temp{0, 0, 0};
        temp.x = this->x + R.x;
        temp.y = this->y + R.y;
        temp.z = this->z + R.z;
        return temp;
    }

    Point_3 operator-(const Point_3 &R) const {
        Point_3 temp{0, 0, 0};
        temp.x = this->x - R.x;
        temp.y = this->y - R.y;
        temp.z = this->z - R.z;
        return temp;
    }

    bool operator==(const Point_3 &R) {
        if (this->x == R.x && this->y == R.y && this->z == R.z) {
            return true;
        }
        return false;
    }

    friend bool operator<(const Point_3 &L, const Point_3 &R) {
        if (L.x < R.x) {
            return true;
        }
        return false;
    }

    friend bool operator==(const Point_3 &L, const Point_3 &R) {
        if (abs(L.y - R.y) < 0.001 && abs(L.x - R.x) < 0.001) {
            return true;
        }
        return false;
    }

    friend Point_3 abs(const Point_3 &R) {
        Point_3 temp{std::abs(R.x), std::abs(R.y), std::abs(R.z)};
        return temp;
    }

    static Point_3 min_two_point(const Point_3 &L, const Point_3 &R) {
        return {((R.x < L.x) ? R.x : L.x), ((R.y < L.y) ? R.y : L.y), ((R.z < L.z) ? R.z : L.z)};
    }

    static Point_3 max_two_point(Point_3 &L, const Point_3 &R) {
        return {((R.x > L.x) ? R.x : L.x), ((R.y > L.y) ? R.y : L.y), ((R.z > L.z) ? R.z : L.z)};
    }


    Point_3 operator/(const float number) const {
        Point_3 temp{0, 0, 0};
        temp.x = this->x / number;
        temp.y = this->y / number;
        temp.z = this->z / number;
        return temp;
    }

    Point_3 operator*(const float number) const {
        Point_3 temp{0, 0, 0};
        temp.x = this->x * number;
        temp.y = this->y * number;
        temp.z = this->z * number;
        return temp;
    }


    float single_area(const Point_3 &R);
};


inline float dot(const Point_3 &A, const Point_3 &b) {
    return A.x * b.x + A.y * b.y + A.z * b.z;
}

#endif //HELLO_MAC_POINT_3_H
