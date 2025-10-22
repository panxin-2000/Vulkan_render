#include <vector>

#include "vector_signed_area.h"
//
// Created by 潘鑫 on 2025/10/3.
//
/**
 * 这里是按照角度的顺序来计算
 * 具体的细节可以参考算法导论和另一本计算机图形学的书
 * 《computational geometry algorithms and applications》
 *
 * @param new_segments
 * @return
 */
bool convex_hull_in_order_of_angles(std::vector<point_2> &new_segments) {
    // 这里需要做什么呢？
    // 判断长度
    if (new_segments.size() >= 3) {
        point_2 a = new_segments.at(new_segments.size() - 3);
        point_2 b = new_segments.at(new_segments.size() - 2);
        point_2 c = new_segments.at(new_segments.size() - 1);

        point_2 ab = b - a;
        point_2 ac = c - a;
        float area = ab.single_area(ac);
        if (area >= 0) {
            // 那么这里是逆时针
            return true;
        } else {
            // 如果是顺时针，那么就需要删除倒数第二个，然后重新计算
            // 重新计算需要重新调用这个函数本身
            // 参数是什么呢？只需要一个向量就好
            new_segments.erase(new_segments.end() - 2);
            return convex_hull_in_order_of_angles(new_segments);
        }
    } else {
        return false;
    }
}

/**
 * 给出任何一个点集都是可以的
 * 先计算出最下方的点，然后再计算好其他点的角度按照大小排序（arcsin）
 * 然后调用计算具体的convex_hull的方法
 * 返回排序好的convex_hull
 * 可能的的问题在哪里？
 * 1. 这里只是使用了float类型，没有使用模版更新新的类型
 * 2. 应该是有一个关于平行的退化情况，这个并没有去写测试，可能相同的输入会产生不同的结果
 *
 * @param segments
 * @return
 */
std::vector<point_2> &calculate_convex_hull(std::vector<point_2> &segments) {
    struct {
        bool operator()(point_2 a, point_2 b) const {
            if (a.y < b.y) {
                return true;
            } else if (a.y > b.y) {
                return false;
            } else if (a.y == b.y) {
                if (a.x < b.x) {
                    return true;
                } else {
                    return false;
                }
            }
        }
    } customLess;
    std::vector<point_2>::iterator current_min = std::min_element(segments.begin(), segments.end(), customLess);

    point_2 min = *current_min;
    segments.erase(current_min);
    std::sort(segments.begin(), segments.end(),
              [min](point_2 a, point_2 b) {
                  point_2 new_a = a - min;
                  point_2 new_b = b - min;
                  if (std::asin(new_a.y / new_a.x) < std::asin(new_b.y / new_b.x)) {
                      return true;
                  } else {
                      return false;
                  }
              });
    // 是的，之前是有问题，全部排序完成之后还需要将最后一个添加到线段中
    // 将最开始的点也添加到最后，目的是为了防止最后一部分是凹的
    // 然后导致了需要检查一些内容
    if (segments.size() >= 3) {
        segments.push_back(min);
    } // 只有在大于三个的时候，才会去添加，不大于三个的时候是没有办法添加的，
    // 因为添加进入会导致判断角度为零

    std::vector<point_2> *new_segments = new std::vector<point_2>;
    // 直接用new，之后变换为引用
    new_segments->push_back(min);
    // new_segments.push_back(*segments.begin());
    // segments.erase(segments.begin);
    // 上面这两行是可以被注释的，因为只是被增加了，其他并没有问题

    for (auto segment: segments) {
        new_segments->push_back(segment);
        // segments.erase(segments.begin);  // 这里其实应该也并不应该存在，因上面for导致的是有问题的
        convex_hull_in_order_of_angles(*new_segments);
    }
    if (new_segments->at(0) == new_segments->at(new_segments->size() - 1)) {
        new_segments->pop_back();
    }
    return *new_segments;
}
