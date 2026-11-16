//
// Created by 潘鑫 on 2025/11/17.
//

#ifndef TREE_ORDER_H
#define TREE_ORDER_H
#include <vector>
#include <stack>

enum class tree_walk_type {
    inorder_type,
    postorder_type,
    preorder_type
};

template<typename T, typename function>
std::vector<T> *tree_walk_with_stack(T root, std::vector<T> *result,
                                     T nil_ptr_or_index, function get_node,
                                     tree_walk_type tree_walk) {
    std::stack<T> ptr_stack;
    auto current_node = root;
    if (root != nil_ptr_or_index) {
        ptr_stack.push(root);
    }
    while (ptr_stack.empty() == false) {
        if (tree_walk == tree_walk_type::preorder_type) {
            result->push_back(ptr_stack.top()); // 输出
        }
        if (get_node(current_node)->left != nil_ptr_or_index) {
            // 当前左子树不为空时前进
            ptr_stack.push(get_node(current_node)->left); // 入栈
            current_node = get_node(current_node)->left; // 更新为左子树
        } else if (get_node(current_node)->left == nil_ptr_or_index && get_node(current_node)->right !=
                   nil_ptr_or_index) {
            // 左子树为空时，先输出，再去访问右子树
            if (tree_walk == tree_walk_type::inorder_type) {
                result->push_back(current_node); // 输出
            }
            if (tree_walk == tree_walk_type::preorder_type) {
                ptr_stack.push(get_node(current_node)->right);
            }
            ptr_stack.push(get_node(current_node)->right); // 入栈
            current_node = get_node(current_node)->right; // 更新为右子树
        } else if (get_node(current_node)->left == nil_ptr_or_index && get_node(current_node)->left ==
                   nil_ptr_or_index) {
            if (tree_walk == tree_walk_type::inorder_type) {
                result->push_back(current_node); // 输出
            }
            while (ptr_stack.size() >= 2) {
                auto top = ptr_stack.top();
                if (tree_walk == tree_walk_type::postorder_type) {
                    result->push_back(ptr_stack.top()); // 输出
                }
                ptr_stack.pop();

                auto second = ptr_stack.top();
                if (top == get_node(second)->right) {
                    continue;
                }
                if (top == get_node(second)->left) {
                    if (tree_walk == tree_walk_type::inorder_type) {
                        result->push_back(second);
                    }
                    if (get_node(second)->right != nil_ptr_or_index) {
                        ptr_stack.push(get_node(second)->right);
                        current_node = get_node(second)->right; // 更新为右子树
                        break;
                    }
                    continue;
                }
            }
            if (ptr_stack.size() == 1) {
                if (tree_walk == tree_walk_type::postorder_type) {
                    result->push_back(ptr_stack.top()); // 输出
                }
                ptr_stack.pop();
            }
        }
    }
    return result;
}


#endif //TREE_ORDER_H
