//
// Created by 潘鑫 on 2025/11/1.
//

#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H
#include "vector_signed_area.h"

template<typename T>
class AABB {
public:
    T min_point; // 最小点，并不一定是真实存在的点，可能是由两个点拼出来的一个点
    T max_point; // 最大点也是一样的


    AABB() {
        min_point = T::int_max_limit(min_point);
        max_point = T::int_min_limit(max_point);
    }
};

#endif //BOUNDING_BOX_H
