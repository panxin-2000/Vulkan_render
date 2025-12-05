//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_ORIENTED_BOUNDING_BOXES_H
#define HELLO_MAC_ORIENTED_BOUNDING_BOXES_H


template<typename T>
class OBB {
public:
    T centroid; // 重心
    T direction; // 方向
    T interval; // 间隔
};
#endif //HELLO_MAC_ORIENTED_BOUNDING_BOXES_H
