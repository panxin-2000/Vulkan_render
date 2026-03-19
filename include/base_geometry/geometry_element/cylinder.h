//
// Created by 潘鑫 on 2025/12/5.
//

#ifndef HELLO_MAC_CYLINDER_H
#define HELLO_MAC_CYLINDER_H
#include "segment.h"


template<typename T>
class cylinder {
public:
    Segment<T> segment;
    float radius;
};
#endif //HELLO_MAC_CYLINDER_H
