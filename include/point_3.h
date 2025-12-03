//
// Created by 潘鑫 on 2025/12/4.
//

#ifndef HELLO_MAC_POINT_3_H
#define HELLO_MAC_POINT_3_H

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


#endif //HELLO_MAC_POINT_3_H
