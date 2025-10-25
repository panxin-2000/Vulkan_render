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

    point_2() {
    }

    point_2(float x1, float y1);


    point_2 operator+(const point_2 &R) const {
        point_2 temp{0, 0};
        temp.x = this->x + R.x;
        temp.y = this->y + R.y;
        return temp;
    }

    bool operator==(const point_2 &R);


    point_2 operator-(const point_2 &R);

    float single_area(const point_2 &R);
};

bool operator==(const point_2 &L, const point_2 &R);

bool operator<(const point_2 &L, const point_2 &R);


typedef point_2 triangle_position;

struct triangle {
    point_2 a;
    point_2 b;
    point_2 c;

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
        if (left.x < right.x) {
            return true;
        }
        if (left.y < right.y) {
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

    // 这里如果想要排序，那么也是有点不太一样的需求的
    // 需要比较它们的重心，其实也不是非要比较重心，比较三个值相加也是可以的。
    //
};

bool operator==(const triangle &L, const triangle &R);


bool on_segment_bounding_box(const point_2 &segment_start_point, point_2 &segment_end_point,
                             point_2 &test_point);

struct segment_position {
    point_2 start_point;
    point_2 end_point;

    bool intersection(struct segment_position &R_segment_position);

    bool get_intersection_point(struct segment_position &R_segment_position, point_2 *result);
};

bool convex_hull_in_order_of_angles(std::vector<point_2> &new_segments);

std::vector<point_2> &calculate_convex_hull(std::vector<point_2> &segments);
#endif //VECTOR_SIGNED_AREA_H
