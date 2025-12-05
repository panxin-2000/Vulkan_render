//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_SPHERES_BOUNDING_VOLUME_H
#define HELLO_MAC_SPHERES_BOUNDING_VOLUME_H

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

#endif //HELLO_MAC_SPHERES_BOUNDING_VOLUME_H
