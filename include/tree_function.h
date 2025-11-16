//
// Created by 潘鑫 on 2025/10/10.
//

#ifndef TREE_FUNCTION_H
#define TREE_FUNCTION_H


// T2 RB_Tree_Node<segment_vector>   T  segment_vector
// 这里的第二个参数是默认需要带指针的
// 这里需要考虑边界条件，边界条件考虑不到的时候会出现死循环
// 需要考虑数据本身的边界条件
// 我这里改了一下顺序，
template<typename value_type, typename ptr>
ptr tree_find_value(ptr root, value_type data) {
    ptr new_root = root;
    ptr result_node = nullptr;
    while (new_root != nullptr) {
        if (new_root->data == data) {
            return new_root;
        } else if (new_root->data < data) {
            // 这里的前后的顺序，需要与插入时比较相同
            new_root = new_root->right;
        } else {
            new_root = new_root->left;
        }
    }
    return result_node;
}

template<typename T>
std::vector<T> &find_all_leaf_node(T node, std::vector<T> &result) {
    std::queue<T> tem;
    if (node != nullptr) {
        tem.push(node);
    }
    while (!tem.empty()) {
        auto node_tem = tem.front();
        if (node_tem->left != nullptr) {
            tem.push(node_tem->left);
        }
        if (node_tem->right != nullptr) {
            tem.push(node_tem->right);
        }
        if (node_tem->left == nullptr && node_tem->right == nullptr) {
            result.push_back(node_tem); // 是叶子节点才添加到向量中准备之后的输出
        }
        tem.pop();
    }
    return result;
}


template<typename T>
void inorder_tree_walk(T node, std::vector<T> *result) {
    if (node != nullptr) {
        inorder_tree_walk(node->left, result);
        result->push_back(node);
        inorder_tree_walk(node->right, result);
    }
}

template<typename T>
void inorder_tree_walk_with_stack(T root, std::vector<T> *result) {
    std::stack<T> ptr_stack;
    auto current_node = root;
    if (root != nullptr) {
        ptr_stack.push(root);
    }
    while (ptr_stack.empty() == false) {
        if (current_node->left != nullptr) {
            // 当前左子树不为空时前进
            ptr_stack.push(current_node->left); // 入栈
            current_node = current_node->left; // 更新为左子树
        } else if (current_node->left == nullptr && current_node->right != nullptr) {
            // 左子树为空时，先输出，再去访问右子树
            result->push_back(current_node); // 输出
            ptr_stack.push(current_node->right); // 入栈
            current_node = current_node->right; // 更新为右子树
        } else if (current_node->left == nullptr && current_node->left == nullptr) {
            result->push_back(current_node); // 输出
            while (ptr_stack.size() >= 2) {
                auto top = ptr_stack.top();
                ptr_stack.pop();
                auto second = ptr_stack.top();
                if (top == second->right) {
                    continue;
                }
                if (top == second->left) {
                    result->push_back(second);
                    if (second->right != nullptr) {
                        ptr_stack.push(second->right);
                        current_node = second->right; // 更新为右子树
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
}

template<typename T>
void preorder_tree_walk(T node, std::vector<T> *result) {
    if (node != nullptr) {
        result->push_back(node);
        preorder_tree_walk(node->left, result);
        preorder_tree_walk(node->right, result);
    }
}

template<typename T>
void preorder_tree_walk_with_stack(T root, std::vector<T> *result) {
    std::stack<T> ptr_stack;
    auto current_node = root;
    if (root != nullptr) {
        ptr_stack.push(root);
    }
    while (ptr_stack.empty() == false) {
        result->push_back(ptr_stack.top()); // 输出
        if (current_node->left != nullptr) {
            // 当前左子树不为空时前进
            ptr_stack.push(current_node->left); // 入栈
            current_node = current_node->left; // 更新为左子树
        } else if (current_node->left == nullptr && current_node->right != nullptr) {
            // 左子树为空时，先输出，再去访问右子树
            ptr_stack.push(current_node->right);
            current_node = current_node->right; // 更新为右子树
        } else if (current_node->left == nullptr && current_node->left == nullptr) {
            while (ptr_stack.size() >= 2) {
                auto top = ptr_stack.top();
                ptr_stack.pop();
                auto second = ptr_stack.top();
                if (top == second->right) {
                    continue;
                }
                if (top == second->left) {
                    if (second->right != nullptr) {
                        ptr_stack.push(second->right);
                        current_node = second->right; // 更新为右子树
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
}

template<typename T>
T preorder_tree_walk_with_stack_find_father(T root, T node) {
    std::stack<T> ptr_stack;
    auto current_node = root;
    if (root != nullptr) {
        ptr_stack.push(root);
    }
    while (ptr_stack.empty() == false) {
        if (current_node->left != nullptr) {
            // 当前左子树不为空时前进
            ptr_stack.push(current_node->left); // 入栈
            current_node = current_node->left; // 更新为左子树
        } else if (current_node->left == nullptr && current_node->right != nullptr) {
            // 左子树为空时，先输出，再去访问右子树
            ptr_stack.push(current_node->right);
            current_node = current_node->right; // 更新为右子树
        } else if (current_node->left == nullptr && current_node->left == nullptr) {
            while (ptr_stack.size() >= 2) {
                auto top = ptr_stack.top();
                ptr_stack.pop();
                auto second = ptr_stack.top();
                if (top == node) {
                    return second;
                }
                if (top == second->right) {
                    continue;
                }
                if (top == second->left) {
                    if (second->right != nullptr) {
                        ptr_stack.push(second->right);
                        current_node = second->right; // 更新为右子树
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
    return nullptr;
}

template<typename T>
void postorder_tree_walk(T node, std::vector<T> *result) {
    if (node != nullptr) {
        postorder_tree_walk(node->left, result);
        postorder_tree_walk(node->right, result);
        result->push_back(node);
    }
}

template<typename T>
T preorder_tree_walk_find_father(T root, T node) {
    if (root != nullptr) {
        if (root->left == node) {
            return root;
        }
        if (root->right == node) {
            return root;
        }
        auto left_temp = preorder_tree_walk_find_father(root->left, node);
        if (nullptr != left_temp) {
            return left_temp;
        }
        auto right_temp = preorder_tree_walk_find_father(root->right, node);
        if (nullptr != right_temp) {
            return right_temp;
        }
    }
    return nullptr;
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
void postorder_tree_walk_with_stack(T root, std::vector<T> *result) {
    std::stack<T> ptr_stack;
    auto current_node = root;
    if (root != nullptr) {
        ptr_stack.push(root);
    }
    while (ptr_stack.empty() == false) {
        if (current_node->left != nullptr) {
            // 当前左子树不为空时前进
            ptr_stack.push(current_node->left); // 入栈
            current_node = current_node->left; // 更新为左子树
        } else if (current_node->left == nullptr && current_node->right != nullptr) {
            // 左子树为空时，先输出，再去访问右子树
            ptr_stack.push(current_node->right); // 入栈
            current_node = current_node->right; // 更新为右子树
        } else if (current_node->left == nullptr && current_node->left == nullptr) {
            while (ptr_stack.size() >= 2) {
                auto top = ptr_stack.top();
                result->push_back(ptr_stack.top()); // 输出
                ptr_stack.pop();

                auto second = ptr_stack.top();
                if (top == second->right) {
                    continue;
                }
                if (top == second->left) {
                    if (second->right != nullptr) {
                        ptr_stack.push(second->right);
                        current_node = second->right; // 更新为右子树
                        break;
                    }
                    continue;
                }
            }
            if (ptr_stack.size() == 1) {
                result->push_back(ptr_stack.top()); // 输出
                ptr_stack.pop();
            }
        }
    }
}

template<typename T>
std::vector<T> level_tree_walk(T node, const std::vector<T> *result) {
    std::queue<T> tem;
    if (node != nullptr) {
        tem.push(node);
    }
    while (!tem.empty()) {
        T *node_tem = tem.front();
        if (node_tem->left != nullptr) {
            tem.push(node_tem->left);
        }
        if (node_tem->right != nullptr) {
            tem.push(node_tem->right);
        }
        result->push_back(node_tem);
        tem.pop();
    }
}

// T2 RB_Tree_Node<segment_vector>   T  segment_vector
// 这个函数中找到的是值，时间上如果能够返回
template<typename T, typename T2>
std::vector<T2> find_interval(T2 root, T &left_node, T &right_node) {
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
