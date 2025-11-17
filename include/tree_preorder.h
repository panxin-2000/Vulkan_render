//
// Created by 潘鑫 on 2025/11/17.
//

#ifndef TREE_PREORDER_H
#define TREE_PREORDER_H


template<typename T>
void preorder_tree_walk(T node, std::vector<T> *result) {
    preorder_tree_walk(node, result,
                       static_cast<T>(nullptr),
                       [](T insert_node) { return *insert_node; });
}


template<typename T, typename function>
void preorder_tree_walk(T node, std::vector<T> *result, T nil_ptr_or_index, function get_node) {
    if (node != nil_ptr_or_index) {
        result->push_back(node);
        preorder_tree_walk(get_node(node).left, result);
        preorder_tree_walk(get_node(node).right, result);
    }
}

template<typename T>
std::vector<T> *preorder_tree_walk_with_stack(T root, std::vector<T> *result) {
    return preorder_tree_walk_with_stack(root, result,
                                         static_cast<T>(nullptr),
                                         [](T insert_node) { return *insert_node; });
}

template<typename T, typename function>
std::vector<T> *preorder_tree_walk_with_stack(T root, std::vector<T> *result, T nil_ptr_or_index, function get_node) {
    std::stack<T> ptr_stack;
    auto current_node = root;
    if (root != nil_ptr_or_index) {
        ptr_stack.push(root);
    }
    while (ptr_stack.empty() == false) {
        result->push_back(ptr_stack.top()); // 输出
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
    return result;
}


#endif //TREE_PREORDER_H
