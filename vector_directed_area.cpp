//
// Created by 潘鑫 on 2025/6/15.
//

#include "gtest/gtest.h"

class segment_vector {
public:
    float x;
    float y;

    segment_vector(float x1, float y1) {
        x = x1;
        y = y1;
    }

    segment_vector operator+(const segment_vector &R) {
        segment_vector temp{0, 0};
        temp.x = this->x + R.x;
        temp.y = this->y + R.y;
        return temp;
    }

    segment_vector operator-(const segment_vector &R) {
        segment_vector temp{0, 0};
        temp.x = this->x - R.x;
        temp.y = this->y - R.y;
        return temp;
    }

    /**
     * 可以用来判断顺时针还是逆时针，第二个相对于第一个逆时针为正，顺时针为负
     * 这里写完还是很简单的
     * @param R
     * @return
     */
    float area(const segment_vector &R) {
        return this->x * R.y - this->y * R.x;
    }
};

typedef segment_vector triangle_position;

struct triangle {
    triangle_position a;
    triangle_position b;
    triangle_position c;
};

bool on_segment(const segment_vector &a, segment_vector &b, segment_vector &c) {
    if (std::min(a.x, b.x) <= c.x &&
        c.x <= std::max(a.x, b.x) &&
        std::min(a.y, b.y) <= c.y &&
        c.y <= std::max(a.y, b.y))
        return true;
    return false;
}

struct segment_position {
    segment_vector start_point;
    segment_vector end_point;


    bool intersection(struct segment_position &R_segment_position) {
        segment_vector ab = this->end_point - this->start_point;
        segment_vector ac = R_segment_position.start_point - this->start_point;
        segment_vector ad = R_segment_position.end_point - this->start_point;

        segment_vector cd = R_segment_position.end_point - R_segment_position.end_point;
        segment_vector ca = this->start_point - R_segment_position.start_point;
        segment_vector cb = this->end_point - R_segment_position.start_point;

        // ac ad 在 ab 的 不同侧的边 且  ca cb 在 cd 的不同侧的边
        float f1 = ab.area(ac);
        float f2 = ab.area(ad);
        float f3 = cd.area(ca);
        float f4 = cd.area(cb);
        if (f1 * f2 < 0 && f3 * f4 > 0) {
            // 这个应该是一个比较简单的判断了 // 算法导论上的比较符号太多了
            return true;
        }
        // 如果有任何一个等于零的时候，那么需要判断是否在线上，因为不在线上也可能为零
        if (f1 == 0 && on_segment(this->start_point, this->end_point, R_segment_position.start_point))
            return true;
        if (f2 == 0 && on_segment(this->start_point, this->end_point, R_segment_position.end_point))
            return true;
        if (f3 == 0 && on_segment(R_segment_position.start_point, R_segment_position.end_point, this->start_point))
            return true;
        if (f3 == 0 && on_segment(R_segment_position.start_point, R_segment_position.end_point, this->end_point))
            return true;
        return false;
    }
};


// 顺时针
TEST(triangle, fsd) { {
        segment_vector ab{1, 1};
        segment_vector ac{2, 0};
        float area = ab.area(ac);
        EXPECT_GT(0, area);
    } {
        segment_vector ab{-10, -1};
        segment_vector ac{-2, -0};
        float area = ab.area(ac);
        EXPECT_GT(0, area);
    }
}

//逆时针
TEST(triangle, fdsd) { {
        segment_vector ab{2, 0};
        segment_vector ac{1, 1};
        float area = ab.area(ac);
        EXPECT_LT(0, area);
    } {
        segment_vector ab{1, 1};
        segment_vector ac{2, 3};
        float area = ab.area(ac);
        EXPECT_LT(0, area);
    }
}

/**
 * 有三个点，分别是1，1    5，5    5，1
 * 5，1 在另外两个点的下方
 */
TEST(triangle, three_point) {
    segment_vector point_a{1, 1};
    segment_vector point_b{5, 5};
    segment_vector point_c{5, 1};
    segment_vector ab = point_b - point_a;
    segment_vector ac = point_c - point_a;
    segment_vector bc = point_c - point_b;
    EXPECT_GT(0, ab.area(ac));
    EXPECT_GT(0, ab.area(bc));
    EXPECT_EQ(ab.area(ac), ab.area(bc));
    // 三角形的三个点，只要第三个点在另外两个点的逆时针方向，那么这个三角形就是逆时针的三角形
    // 所以可以通过这个办法来简单的判断顺时针和逆时针，确实只有在做的时候才会更加了解具体相关的细节
}
