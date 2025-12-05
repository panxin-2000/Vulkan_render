//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_SPHERE_SWEPT_VOLUME_H
#define HELLO_MAC_SPHERE_SWEPT_VOLUME_H
#include "iostream"

template<typename T>
class Sphere_sweep_line {
public:
    T center; // 球心
    float radius; // 半径

    Sphere() {
    }

    Sphere(std::initializer_list<T> points) {
    }

    Sphere(std::vector<T> &points) {
    }
};

template<typename T>
class Sphere_sweep_AABB {
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

template<typename T>
class Sphere_sweep_OBB {
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
