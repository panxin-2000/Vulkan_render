//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_SPHERES_BOUNDING_VOLUME_H
#define HELLO_MAC_SPHERES_BOUNDING_VOLUME_H
#include <cstdlib>

template<typename T>
class Sphere {
public:
    T center; // 球心
    float radius; // 半径

    Sphere(T center, float radius) : center(center), radius(radius) {
    }

    Sphere(std::initializer_list<T> points) {
        if (points.size() == 2) {
            auto it = points.begin();
            center = *it;
            it++;
            radius = *it;
        }
    }

    Sphere(std::vector<T> &points) {
    }
};


// 二维其实是不需要法向量的(所以还是可以用球进行直接表示)，三维中表示才是需要的
class Circle_3 {
public:
    Point_3 center; // 球心
    Point_3 normal;
    float radius; // 半径
};

#endif //HELLO_MAC_SPHERES_BOUNDING_VOLUME_H
