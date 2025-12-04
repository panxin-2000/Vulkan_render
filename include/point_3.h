//
// Created by 潘鑫 on 2025/12/4.
//

#ifndef HELLO_MAC_POINT_3_H
#define HELLO_MAC_POINT_3_H

class Point_3 {
public:
    float x;
    float y;
    float z;
    using point_type = Point_3;


    Point_3() {
    }

    Point_3(float x1, float y1, float z1) {
        x = x1;
        y = y1;
        z = z1;
    }

    Point_3 operator+(const Point_3 &R) const {
        Point_3 temp{0, 0, 0};
        temp.x = this->x + R.x;
        temp.y = this->y + R.y;
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

    Point_3 operator-(const Point_3 &R);

    float single_area(const Point_3 &R);
};


#endif //HELLO_MAC_POINT_3_H
