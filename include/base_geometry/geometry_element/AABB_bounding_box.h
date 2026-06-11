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
    T min_point_; // 最小点，并不一定是真实存在的点，可能是由两个点拼出来的一个点
    T max_point_; // 最大点也是一样的

    AABB_min_max(AABB_centroid<T> box) {
        min_point_ = box.centroid_point_ - box.direction_interval_;
        max_point_ = box.centroid_point_ + box.direction_interval_;
    }

    AABB_min_max() {
        min_point_ = T::init_max_limit();
        max_point_ = T::init_min_limit();
    }

    AABB_min_max(std::initializer_list<T> points) {
        min_point_ = T::init_max_limit();
        max_point_ = T::init_min_limit();
        for (auto vertex_point: points) {
            min_point_ = T::min_two_point(min_point_, vertex_point);
            max_point_ = T::max_two_point(max_point_, vertex_point);
        }
    }

    AABB_min_max(std::vector<T> &points) {
        min_point_ = T::init_max_limit();
        max_point_ = T::init_min_limit();
        for (auto vertex_point: points) {
            min_point_ = T::min_two_point(min_point_, vertex_point);
            max_point_ = T::max_two_point(max_point_, vertex_point);
        }
    }

    AABB_min_max(T a_points, T b_points) {
        min_point_ = T::min_two_point(a_points, b_points);
        max_point_ = T::max_two_point(a_points, b_points);
    }

    AABB_min_max(T a_points, T b_points, T c_points) {
        min_point_ = T::min_two_point(a_points, b_points);
        max_point_ = T::max_two_point(a_points, b_points);
        min_point_ = T::min_two_point(min_point_, c_points);
        max_point_ = T::max_two_point(max_point_, c_points);
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
    T centroid_point_;     // 重心
    T direction_interval_; // 方向间隔

    AABB_centroid() = default;

    auto get_centroid_point() {
        return centroid_point_;
    }

    auto get_radius() {
        return direction_interval_;
    }

    void add_offset(T offset) {
        centroid_point_ = centroid_point_ + offset;
    }

    explicit AABB_centroid(AABB_min_max<T> box) {
        centroid_point_     = (box.min_point_ + box.max_point_) / 2;
        direction_interval_ = (box.max_point_ - box.min_point_) / 2;
    }


    /**
     *
     * @param l_points 直接给值时是中心点
     * @param r_points 直接给值时是每个方向的大小，(半径)
     */
    AABB_centroid(T l_points, T r_points) {
        centroid_point_     = l_points;
        direction_interval_ = r_points;
        direction_interval_ = abs(direction_interval_);
    }

    AABB_centroid<T> &operator =(AABB_min_max<T> box) {
        centroid_point_     = (box.min_point_ + box.max_point_) / 2;
        direction_interval_ = (box.max_point_ - box.min_point_) / 2;
        return *this;
    }
};


#endif //BOUNDING_BOX_H
