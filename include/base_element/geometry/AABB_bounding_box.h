//
// Created by 潘鑫 on 2025/11/1.
//

#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H
#include "iostream"

template<typename T>
class AABB_centroid;

template<typename T>
class AABB_min_max;

template<typename T>
class AABB_min_max {
public:
    T min_point; // 最小点，并不一定是真实存在的点，可能是由两个点拼出来的一个点
    T max_point; // 最大点也是一样的

    AABB_min_max(AABB_centroid<T> box) {
        min_point = box.centroid_point - box.direction_interval;
        max_point = box.centroid_point + box.direction_interval;
    }

    AABB_min_max() {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
    }

    AABB_min_max(std::initializer_list<T> points) {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
        for (auto vertex_point: points) {
            min_point = T::min_two_point(min_point, vertex_point);
            max_point = T::max_two_point(max_point, vertex_point);
        }
    }

    AABB_min_max(std::vector<T> &points) {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
        for (auto vertex_point: points) {
            min_point = T::min_two_point(min_point, vertex_point);
            max_point = T::max_two_point(max_point, vertex_point);
        }
    }

    AABB_min_max(T l_points, T r_points) {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
        min_point = T::min_two_point(min_point, l_points);
        max_point = T::max_two_point(max_point, l_points);
        min_point = T::min_two_point(min_point, r_points);
        max_point = T::max_two_point(max_point, r_points);
    }


    /**
     * 在包围盒的内部和边缘的线上都 返回 true
     * @param box
     * @param test_point 需要测试 是否 在包围盒内的点
     * @return
     */
};

template<typename T>
class AABB_centroid {
public:
    T centroid_point; // 重心
    T direction_interval; // 方向间隔

    AABB_centroid() {
        centroid_point = 0;
        direction_interval = 0;
    }

    AABB_centroid(AABB_min_max<T> box) {
        centroid_point = (box.min_point + box.max_point) / 2;
        direction_interval = (box.max_point - box.min_point) / 2;
    }


    AABB_centroid(std::initializer_list<T> points) {
        T min_point = T::int_max_limit(min_point); // 这行有点奇怪，看起来感觉逻辑有问题
        T max_point = T::int_min_limit(max_point);
        for (auto vertex_point: points) {
            min_point = T::min_two_point(min_point, vertex_point);
            max_point = T::max_two_point(max_point, vertex_point);
        }
        centroid_point = (min_point + max_point) / 2;
        direction_interval = (max_point - min_point) / 2;
    }

    AABB_centroid(std::vector<T> &points) {
        T min_point = T::int_max_limit(min_point);
        T max_point = T::int_min_limit(max_point);
        for (auto vertex_point: points) {
            min_point = T::min_two_point(min_point, vertex_point);
            max_point = T::max_two_point(max_point, vertex_point);
        }
        centroid_point = (min_point + max_point) / 2;
        direction_interval = (max_point - min_point) / 2;
    }

    AABB_centroid(T l_points, T r_points) {
        T min_point = T::int_max_limit(min_point);
        T max_point = T::int_min_limit(max_point);
        min_point = T::min_two_point(min_point, l_points);
        max_point = T::max_two_point(max_point, l_points);
        min_point = T::min_two_point(min_point, r_points);
        max_point = T::max_two_point(max_point, r_points);
        centroid_point = (min_point + max_point) / 2;
        direction_interval = (max_point - min_point) / 2;
    }
};


#endif //BOUNDING_BOX_H
