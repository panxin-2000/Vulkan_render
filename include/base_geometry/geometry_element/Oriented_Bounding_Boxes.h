//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_ORIENTED_BOUNDING_BOXES_H
#define HELLO_MAC_ORIENTED_BOUNDING_BOXES_H


template<typename T>
class OBB_2D {
public:
    T centroid_{};       // 重心
    T direction_1{1, 0}; // 方向
    T direction_2{0, 1}; // 方向
    T interval_{};       // 间隔 半高

    OBB_2D(const T centroid, const T interval) : centroid_(centroid), interval_(interval) {
    }

    OBB_2D(const T centroid, const T direction_a, const T direction_b,
           const T interval) : centroid_(centroid),
                               direction_1(direction_a),
                               direction_2(direction_b),
                               interval_(interval) {
    }

    explicit OBB_2D(std::vector<T> &points) {
    }
};

// class OBB_3D {
// public:
// using T = Point_3;
// T centroid_{};          // 重心
// T direction_1{1, 0, 0}; // 方向
// T direction_2{0, 1, 0}; // 方向
// T interval_{};          // 间隔


// OBB_3D(const T centroid, const T interval) : centroid_(centroid), interval_(interval) {
// }

// OBB_3D(const T centroid, const T direction_a, const T direction_b, const T direction_c,
// const T interval) : centroid_(centroid),
// direction_1(direction_a),
// direction_2(direction_b),
// interval_(interval) {
// }

// explicit OBB_3D(std::vector<T> &points) {
// }
// };
#endif //HELLO_MAC_ORIENTED_BOUNDING_BOXES_H
