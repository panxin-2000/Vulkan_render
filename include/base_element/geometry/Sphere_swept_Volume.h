//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_SPHERE_SWEPT_VOLUME_H
#define HELLO_MAC_SPHERE_SWEPT_VOLUME_H
#include "iostream"

template<typename T>
class Sphere {
public:
    T center; // 球心
    T radius; // 半径

    Sphere() {
    }

    Sphere(std::initializer_list<T> points) {
    }

    Sphere(std::vector<T> &points) {
    }
};

#endif //HELLO_MAC_SPHERE_SWEPT_VOLUME_H
