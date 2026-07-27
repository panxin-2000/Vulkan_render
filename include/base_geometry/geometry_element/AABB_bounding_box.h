//
// Created by 潘鑫 on 2025/11/1.
//

#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H
#include <Eigen/Eigen>

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

    T get_centroid() const {
        return (min_point_ + max_point_) / 2;
    }

    AABB_min_max() {
        min_point_ = T::init_max_limit();
        max_point_ = T::init_min_limit();
    }

    AABB_min_max<T> add_offset(T offset) {
        return {min_point_ + offset, max_point_ + offset};
    }

    AABB_min_max(std::initializer_list<T> points) {
        min_point_ = T::init_max_limit();
        max_point_ = T::init_min_limit();
        for (auto vertex_point: points) {
            min_point_ = T::min(min_point_, vertex_point);
            max_point_ = T::max(max_point_, vertex_point);
        }
    }

    AABB_min_max(std::vector<T> &points) {
        min_point_ = T::init_max_limit();
        max_point_ = T::init_min_limit();
        for (auto vertex_point: points) {
            min_point_ = T::min(min_point_, vertex_point);
            max_point_ = T::max(max_point_, vertex_point);
        }
    }

    AABB_min_max(T a_points, T b_points) {
        min_point_ = T::min(a_points, b_points);
        max_point_ = T::max(a_points, b_points);
    }

    AABB_min_max(T a_points, T b_points, T c_points) {
        min_point_ = T::min(a_points, b_points);
        max_point_ = T::max(a_points, b_points);
        min_point_ = T::min(min_point_, c_points);
        max_point_ = T::max(max_point_, c_points);
    }

    AABB_min_max<T> multiply_matrix(const Eigen::Matrix4f &matrix) {
        if constexpr (std::is_same_v<T, Point_3>) {
            const Eigen::Vector4f min_point(min_point_.x, min_point_.y, min_point_.z, 0);
            const Eigen::Vector4f max_point(max_point_.x, max_point_.y, max_point_.z, 0);
            Eigen::Vector4f new_min = matrix * min_point;
            Eigen::Vector4f new_max = matrix * max_point;
            AABB_min_max<T> result  = AABB_min_max<T>{
                {new_min.x(), new_min.y(), new_min.z()},
                {new_max.x(), new_max.y(), new_max.z()}
            };
            return result;
        } else if constexpr (std::is_same_v<T, Point_2>) {
            AABB_min_max<T> result{{0, 0, 0}, {0, 0, 0}};
            return result;
        }
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

    auto get_centroid() const {
        return centroid_point_;
    }

    auto get_radius() const {
        return direction_interval_;
    }

    void add_offset(T offset) {
        centroid_point_ = centroid_point_ + offset;
    }

    explicit AABB_centroid(AABB_min_max<T> box) {
        centroid_point_     = (box.min_point_ + box.max_point_) / 2;
        direction_interval_ = (box.max_point_ - box.min_point_) / 2;
    }

    AABB_centroid<T> multiply_matrix(const Eigen::Matrix4f &matrix) const {
        if constexpr (std::is_same_v<T, Point_3>) {
            const Eigen::Vector4f centroid(centroid_point_.x, centroid_point_.y, centroid_point_.z, 1.0f);
            const Eigen::Vector3f direction(direction_interval_.x, direction_interval_.y, direction_interval_.z);
            Eigen::Vector4f new_centroid  = matrix * centroid;
            Eigen::Matrix3f R = matrix.block<3, 3>(0, 0);
            Eigen::Vector3f new_direction = R.cwiseAbs() * direction;
            AABB_centroid<T> result{
                {new_centroid.x(), new_centroid.y(), new_centroid.z()},
                {new_direction.x(), new_direction.y(), new_direction.z()}
            };
            return result;
        } else if constexpr (std::is_same_v<T, Point_2>) {
            AABB_centroid<T> result{{0, 0}, {0, 0}};
            return result;
        }
    }


    /**
     *
     * @param centroid_point 直接给值时是中心点
     * @param direction_interval 直接给值时是每个方向的大小，(半径)
     */
    AABB_centroid(T centroid_point, T direction_interval) {
        centroid_point_     = centroid_point;
        direction_interval_ = direction_interval;
        direction_interval_ = abs(direction_interval_);
    }

    AABB_centroid<T> &operator =(AABB_min_max<T> box) {
        centroid_point_     = (box.min_point_ + box.max_point_) / 2;
        direction_interval_ = abs(box.max_point_ - box.min_point_) / 2;
        return *this;
    }
};


#endif //BOUNDING_BOX_H
