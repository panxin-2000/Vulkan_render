//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_BASE_H
#define HELLO_MAC_BASE_H


// 需要想办法，能够按照顺序将全部的头文件都放置在这里
#include "geometry_element/point_2.h"
#include "geometry_element/point_3.h"
#include "geometry_element/AABB_bounding_box.h"
#include "geometry_element/ray.h"
#include "geometry_element/eight_DOP.h"
#include "geometry_element/Oriented_Bounding_Boxes.h"
#include "geometry_element/plane.h"
#include "geometry_element/polygon.h"
#include "geometry_element/sphere_bounding_volume.h"
#include "geometry_element/straight_line.h"
#include "geometry_element/cylinder.h"
#include "geometry_element/segment.h"
#include "geometry_element/straight_line.h"
#include "geometry_element/AABB_bounding_box.h"
#include "geometry_element/trapezoid.h"
#include "geometry_element/triangle.h"


template<typename T>
inline T mean(const std::vector<T> &points) {
    T temp_point = {0, 0};
    for (const auto point: points) {
        temp_point += point;
    }
    temp_point = temp_point / static_cast<float>(points.size());
    return temp_point;
}



template<typename T>
inline auto Covariance_Matrix(const std::vector<T> &points) {


}


#endif //HELLO_MAC_BASE_H
