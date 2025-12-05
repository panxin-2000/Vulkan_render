//
// Created by 潘鑫 on 2025/11/1.
//

#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

template<typename T>
class AABB {
public:
    T min_point; // 最小点，并不一定是真实存在的点，可能是由两个点拼出来的一个点
    T max_point; // 最大点也是一样的


    AABB() {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
    }

    AABB(std::initializer_list<T> points) {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
        for (auto vertex_point: points) {
            min_point = T::min_two_point(min_point, vertex_point);
            max_point = T::max_two_point(max_point, vertex_point);
        }
    }

    AABB(std::vector<T> &points) {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
        for (auto vertex_point: points) {
            min_point = T::min_two_point(min_point, vertex_point);
            max_point = T::max_two_point(max_point, vertex_point);
        }
    }

    AABB(T l_points, T r_points) {
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


#endif //BOUNDING_BOX_H
