//
// Created by 潘鑫 on 2026/8/10.
//

#ifndef HELLO_MAC_NON_UNIFORM_RATIONAL_B_SPLINE_H
#define HELLO_MAC_NON_UNIFORM_RATIONAL_B_SPLINE_H
#include "B_spline_cureve.h"

template<typename T>
class NURBS : public B_spline<T> {

    // 比 B_spline 多了 一个系数, 利用这个系数 单独做一条 B_spline
    // 两个 B_spline 分别求出来, 之后在 想除 ,得到最后的结果 


};


#endif //HELLO_MAC_NON_UNIFORM_RATIONAL_B_SPLINE_H
