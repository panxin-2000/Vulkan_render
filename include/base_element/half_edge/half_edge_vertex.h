//
// Created by 潘鑫 on 2025/12/13.
//

#ifndef HELLO_MAC_HALF_EDGE_VERTEX_H
#define HELLO_MAC_HALF_EDGE_VERTEX_H

#include "../geometry/point_2.h"
#include "base_element/point_3.h"

struct vertex_xy : public Point_2 {
    using point_type = Point_2;

    int incident_half_edge;

    vertex_xy(float x1, float y1) : Point_2(x1, y1) {
    }

    vertex_xy(Point_2 point) : Point_2(point.x, point.y) {
    }

    vertex_xy(float x1, float y1, int incident_half_edge_1) : Point_2(x1, y1) {
        incident_half_edge = incident_half_edge_1;
    }

    vertex_xy operator/(unsigned long number) const {
        const vertex_xy result(x / number, y / number);
        return result;
    }

    bool operator<(const vertex_xy &right) const {
        if (y < right.y) {
            // 先比较x轴，x轴小的为小
            return true;
        } else if (y == right.y && x < right.x) {
            // 之后再比较y轴，y轴小的为小
            return true;
        } else if (x == right.x && y == right.y) {
            if (incident_half_edge % 2 == 0) {
                // 值完全一样，比较是否是起点，是起点的边，
                return true; // a起点，b不是起点，a小，a不是起点，那么b是不是起点都在a前，没什么关系
            }
        }
        return false;
    }

    bool operator==(const vertex_xy &right) const {
        if (x == right.x && y == right.y && incident_half_edge == right.incident_half_edge) {
            return true;
        }
        return false;
    }
};


struct vertex_xyz : public Point_3 {
    using point_type = Point_3;
    int incident_half_edge;

    vertex_xyz(float x1, float y1, float z1) : Point_3(x1, y1, z1) {
    }

    bool operator<(const vertex_xyz &right) const {
        if (y < right.y) {
            // 先比较x轴，x轴小的为小
            return true;
        } else if (y == right.y && x < right.x) {
            // 之后再比较y轴，y轴小的为小
            return true;
        } else if (x == right.x && y == right.y) {
            if (incident_half_edge % 2 == 0) {
                // 值完全一样，比较是否是起点，是起点的边，
                return true; // a起点，b不是起点，a小，a不是起点，那么b是不是起点都在a前，没什么关系
            }
        }
        return false;
    }

    bool operator==(const vertex_xyz &right) const {
        if (x == right.x && y == right.y && incident_half_edge == right.incident_half_edge) {
            return true;
        }
        return false;
    }
};


#endif //HELLO_MAC_HALF_EDGE_VERTEX_H
