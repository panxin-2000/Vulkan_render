//
// Created by 潘鑫 on 2025/10/10.
//

#ifndef TREE_FUNCTION_H
#define TREE_FUNCTION_H


// T2 RB_Tree_Node<segment_vector>   T  segment_vector
// 这里的第二个参数是默认需要带指针的
template<typename T, typename T2>
T2 tree_find_value(T2 root, T data) {
    T2 new_root = root;
    T2 result_node = nullptr;
    while (new_root != nullptr) {
        if (new_root->data < data) {
            new_root = new_root->right;
        } else if (data < new_root->data) {
            new_root = new_root->left;
        } else {
            return new_root;
        }
    }
    return result_node;
}


// T2 RB_Tree_Node<segment_vector>   T  segment_vector
template<typename T, typename T2>
std::vector<T2 *> find_interval(T2 root, T &left_node, T &right_node) {
    //
    auto new_left_node = left_node;
    auto new_right_node = right_node;
    if (right_node < left_node) {
        std::swap(new_left_node, new_right_node);
    }
    // auto root = this;
    auto result = *new std::vector<T2 *>;
    // 找到最小值和最大值
    auto min_node = tree_find_value(&root, new_left_node); //其实这里是稍微有点问题的
    auto max_node = tree_find_value(&root, new_right_node); //大部分情况是是取一个间隔，并不能准确的对应的值
    if (min_node == nullptr || max_node == nullptr) {
        return result;
    }
    // 已经拿到最小值了，最小值的向右都比最小值大
    // 先遍历最小值大右子树，每个都添加到向量中，
    // 不用那么麻烦，找后继就好，直到找到了最大值，
    // 这些值都添加到向量中
    // while (min_node->tree_successor(min_node) != nullptr) {
    //     // 这里稍微有点死循环
    //     auto temp = min_node->tree_successor(min_node);
    //     result.push_back(temp);
    //     min_node = temp;
    //     if (temp == max_node) {
    //         break;
    //     }
    // }
    while (max_node->tree_predecessor(max_node) != nullptr) {
        // 这里稍微有点死循环
        auto temp = max_node->tree_predecessor(max_node);
        result.push_back(temp);
        max_node = temp;
        if (temp == min_node) {
            break;
        }
    }
    return result;
}


#endif //TREE_FUNCTION_H
