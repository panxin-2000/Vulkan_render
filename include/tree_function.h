//
// Created by 潘鑫 on 2025/10/10.
//

#ifndef TREE_FUNCTION_H
#define TREE_FUNCTION_H
#include "tree_inorder.h"
#include "tree_level.h"
#include "tree_postorder.h"
#include "tree_preorder.h"


// T2 RB_Tree_Node<segment_vector>   T  segment_vector
// 这里的第二个参数是默认需要带指针的
// 这里需要考虑边界条件，边界条件考虑不到的时候会出现死循环
// 需要考虑数据本身的边界条件
// 我这里改了一下顺序，
template<typename value_type, typename ptr, typename function>
ptr tree_find_value(ptr root, value_type data, ptr nil_ptr_or_index, function get_node) {
    ptr new_root = root;
    ptr result_node = nil_ptr_or_index;
    while (new_root != nil_ptr_or_index) {
        if (get_node(new_root).data == data) {
            return new_root;
        } else if (get_node(new_root).data < data) {
            // 这里的前后的顺序，需要与插入时比较相同
            new_root = get_node(new_root).right;
        } else {
            new_root = get_node(new_root).left;
        }
    }
    return result_node;
}

template<typename value_type, typename ptr>
ptr tree_find_value(ptr root, value_type data) {
    return tree_find_value(root, data, static_cast<ptr>(nullptr), [](ptr insert_node) { return *insert_node; });
}

template<typename T, typename function>
std::vector<T> &find_all_leaf_node(T node, std::vector<T> &result, T nil_ptr_or_index, function get_node) {
    std::queue<T> tem;
    if (node != nil_ptr_or_index) {
        tem.push(node);
    }
    while (!tem.empty()) {
        auto node_tem = tem.front();
        if (get_node(node_tem).left != nil_ptr_or_index) {
            tem.push(get_node(node_tem).left);
        }
        if (get_node(node_tem).right != nil_ptr_or_index) {
            tem.push(get_node(node_tem).right);
        }
        if (get_node(node_tem).left == nil_ptr_or_index && get_node(node_tem).right == nil_ptr_or_index) {
            result.push_back(node_tem); // 是叶子节点才添加到向量中准备之后的输出
        }
        tem.pop();
    }
    return result;
}

template<typename T>
std::vector<T> &find_all_leaf_node(T node, std::vector<T> &result) {
    return find_all_leaf_node(node, result, static_cast<T>(nullptr), [](T insert_node) { return *insert_node; });
}


template<typename T>
T preorder_tree_walk_with_stack_find_father(T root, T node) {
    preorder_tree_walk_with_stack_find_father(root, node, static_cast<T>(nullptr),
                                              [](T insert_node) { return *insert_node; });
}

template<typename T, typename function>
T preorder_tree_walk_with_stack_find_father(T root, T node, T nil_ptr_or_index, function get_node) {
    std::stack<T> ptr_stack;
    auto current_node = root;
    if (root != nil_ptr_or_index) {
        ptr_stack.push(root);
    }
    while (ptr_stack.empty() == false) {
        if (get_node(current_node).left != nil_ptr_or_index) {
            // 当前左子树不为空时前进
            ptr_stack.push(get_node(current_node).left); // 入栈
            current_node = get_node(current_node).left; // 更新为左子树
        } else if (get_node(current_node).left == nil_ptr_or_index && get_node(current_node).right !=
                   nil_ptr_or_index) {
            // 左子树为空时，先输出，再去访问右子树
            ptr_stack.push(get_node(current_node).right);
            current_node = get_node(current_node).right; // 更新为右子树
        } else if (get_node(current_node).left == nil_ptr_or_index && get_node(current_node).left == nil_ptr_or_index) {
            while (ptr_stack.size() >= 2) {
                auto top = ptr_stack.top();
                ptr_stack.pop();
                auto second = ptr_stack.top();
                if (top == node) {
                    return second;
                }
                if (top == get_node(second).right) {
                    continue;
                }
                if (top == get_node(second).left) {
                    if (get_node(second).right != nil_ptr_or_index) {
                        ptr_stack.push(get_node(second).right);
                        current_node = get_node(second).right; // 更新为右子树
                        break;
                    }
                    continue;
                }
            }
            if (ptr_stack.size() == 1) {
                ptr_stack.pop();
            }
        }
    }
    return nil_ptr_or_index;
}


template<typename T>
T preorder_tree_walk_find_father(T root, T node) {
    return preorder_tree_walk_find_father(root, node,
                                          static_cast<T>(nullptr),
                                          [](T insert_node) { return *insert_node; });
}

template<typename T, typename function>
T preorder_tree_walk_find_father(T root, T node, T nil_ptr_or_index, function get_node) {
    if (root != nil_ptr_or_index) {
        if (get_node(root).left == node) {
            return root;
        }
        if (get_node(root).right == node) {
            return root;
        }
        auto left_temp = preorder_tree_walk_find_father(get_node(root).left, node);
        if (nil_ptr_or_index != left_temp) {
            return left_temp;
        }
        auto right_temp = preorder_tree_walk_find_father(get_node(root).right, node);
        if (nil_ptr_or_index != right_temp) {
            return right_temp;
        }
    }
    return nil_ptr_or_index;
}

template<typename T, typename function>
T find_insert_position(T root, T new_node, T nil_ptr_or_index, function get_node) {
    T new_root = root;
    T insert_node = nil_ptr_or_index;
    while (new_root != nil_ptr_or_index) {
        insert_node = new_root;
        if (get_node(insert_node).data < get_node(new_node).data) {
            // 新插入的结点在比较的后面
            new_root = get_node(new_root).right;
        } else {
            new_root = get_node(new_root).left;
        }
    }
    return insert_node;
}

template<typename T>
T find_insert_position(T root, T new_node) {
    return find_insert_position(root, new_node, static_cast<T>(nullptr), [](T insert_node) { return *insert_node; });
}


// T2 RB_Tree_Node<segment_vector>   T  segment_vector
// 这个函数中找到的是值，时间上如果能够返回
template<typename T, typename T2>
std::vector<T2> find_interval(T2 root, T &left_node, T &right_node) {
    return find_interval(root, left_node, right_node,
                         static_cast<T2>(nullptr),
                         [](T2 insert_node) { return *insert_node; });
}

template<typename T, typename T2, typename function>
std::vector<T2> find_interval(T2 root, T &left_node, T &right_node, T2 nil_ptr_or_index, function get_node) {
    //
    auto new_left_node = left_node;
    auto new_right_node = right_node;
    if (right_node < left_node) {
        std::swap(new_left_node, new_right_node);
    }
    // auto root = this;
    auto result = *new std::vector<T2>;
    // 找到最小值和最大值
    auto min_node = tree_find_value(root, new_left_node); //其实这里是稍微有点问题的
    auto max_node = tree_find_value(root, new_right_node); //大部分情况是是取一个间隔，并不能准确的对应的值
    if (min_node == nil_ptr_or_index || max_node == nil_ptr_or_index) {
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
    while (get_node(max_node).tree_predecessor(max_node) != nil_ptr_or_index) {
        // 这里稍微有点死循环
        auto temp = get_node(max_node).tree_predecessor(max_node);
        result.push_back(temp);
        max_node = temp;
        if (temp == min_node) {
            break;
        }
    }
    return result;
}


template<typename ptr>
static ptr tree_maximum(ptr tree_node) {
    ptr return_node = nullptr;
    while (tree_node != nullptr) {
        return_node = tree_node;
        tree_node = tree_node->right;
    }
    return return_node;
}

template<typename ptr>
static ptr tree_minimum(ptr tree_node) {
    ptr return_node = nullptr;
    while (tree_node != nullptr) {
        return_node = tree_node;
        tree_node = tree_node->left;
    }
    return return_node;
}

/**
 * 这个方法可以从扩展类中移动到基类当中
 * @param root
 * @return
 */
template<typename ptr>
ptr find_miximum_leaf(ptr root) {
    if (root == nullptr) {
        return nullptr;
    } else {
        while (root->right != nullptr || root->left != nullptr) {
            if (root->right != nullptr) {
                return find_miximum_leaf(root->right);
            } else {
                return find_miximum_leaf(root->left);
            }
        }
        return root;
    }
}


#endif //TREE_FUNCTION_H
