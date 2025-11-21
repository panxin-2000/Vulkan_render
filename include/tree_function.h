//
// Created by 潘鑫 on 2025/10/10.
//

#ifndef TREE_FUNCTION_H
#define TREE_FUNCTION_H
#include "tree_inorder.h"
#include "tree_level.h"
#include "tree_postorder.h"
#include "tree_preorder.h"

namespace BIN_tree {
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
            if (get_node(new_root)->data == data) {
                return new_root;
            } else if (get_node(new_root)->data < data) {
                // 这里的前后的顺序，需要与插入时比较相同
                new_root = get_node(new_root)->right;
            } else {
                new_root = get_node(new_root)->left;
            }
        }
        return result_node;
    }
}

template<typename T, typename function>
std::vector<T> &find_all_leaf_node(T node, std::vector<T> &result, T nil_ptr_or_index, function get_node) {
    std::queue<T> tem;
    if (node != nil_ptr_or_index) {
        tem.push(node);
    }
    while (!tem.empty()) {
        auto node_tem = tem.front();
        if (get_node(node_tem)->left != nil_ptr_or_index) {
            tem.push(get_node(node_tem)->left);
        }
        if (get_node(node_tem)->right != nil_ptr_or_index) {
            tem.push(get_node(node_tem)->right);
        }
        if (get_node(node_tem)->left == nil_ptr_or_index && get_node(node_tem)->right == nil_ptr_or_index) {
            result.push_back(node_tem); // 是叶子节点才添加到向量中准备之后的输出
        }
        tem.pop();
    }
    return result;
}


template<typename T, typename function>
T preorder_tree_walk_with_stack_find_father(T root, T node, T nil_ptr_or_index, function get_node) {
    std::stack<T> ptr_stack;
    auto current_node = root;
    if (root != nil_ptr_or_index) {
        ptr_stack.push(root);
    }
    while (ptr_stack.empty() == false) {
        if (get_node(current_node)->left != nil_ptr_or_index) {
            // 当前左子树不为空时前进
            ptr_stack.push(get_node(current_node)->left); // 入栈
            current_node = get_node(current_node)->left; // 更新为左子树
        } else if (get_node(current_node)->left == nil_ptr_or_index && get_node(current_node)->right !=
                   nil_ptr_or_index) {
            // 左子树为空时，先输出，再去访问右子树
            ptr_stack.push(get_node(current_node)->right);
            current_node = get_node(current_node)->right; // 更新为右子树
        } else if (get_node(current_node)->left == nil_ptr_or_index && get_node(current_node)->left ==
                   nil_ptr_or_index) {
            while (ptr_stack.size() >= 2) {
                auto top = ptr_stack.top();
                ptr_stack.pop();
                auto second = ptr_stack.top();
                if (top == node) {
                    return second;
                }
                if (top == get_node(second)->right) {
                    continue;
                }
                if (top == get_node(second)->left) {
                    if (get_node(second)->right != nil_ptr_or_index) {
                        ptr_stack.push(get_node(second)->right);
                        current_node = get_node(second)->right; // 更新为右子树
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


template<typename T, typename function>
T preorder_tree_walk_find_father(T root, T node, T nil_ptr_or_index, function get_node) {
    if (root != nil_ptr_or_index) {
        if (get_node(root)->left == node) {
            return root;
        }
        if (get_node(root)->right == node) {
            return root;
        }
        auto left_temp = preorder_tree_walk_find_father(get_node(root)->left, node);
        if (nil_ptr_or_index != left_temp) {
            return left_temp;
        }
        auto right_temp = preorder_tree_walk_find_father(get_node(root)->right, node);
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
        if (get_node(insert_node)->data < get_node(new_node)->data) {
            // 新插入的结点在比较的后面
            new_root = get_node(new_root)->right;
        } else {
            new_root = get_node(new_root)->left;
        }
    }
    return insert_node;
}

template<typename T, typename T2>
std::vector<T2> find_interval(T2 root, T &left_node, T &right_node) {
    return find_interval(root, left_node, right_node,
                         static_cast<T2>(nullptr),
                         [](T2 insert_node) { return insert_node; });
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
    auto min_node = BIN_tree::tree_find_value(root, new_left_node, nil_ptr_or_index, get_node); //其实这里是稍微有点问题的
    auto max_node = BIN_tree::tree_find_value(root, new_right_node, nil_ptr_or_index, get_node);
    //大部分情况是是取一个间隔，并不能准确的对应的值
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
    while (get_node(max_node)->tree_predecessor(max_node) != nil_ptr_or_index) {
        // 这里稍微有点死循环
        auto temp = get_node(max_node)->tree_predecessor(max_node);
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

template<typename ptr, typename function>
static ptr tree_minimum(ptr tree_node, ptr nil_ptr_or_index, function get_node) {
    ptr return_node = nil_ptr_or_index;
    while (tree_node != nil_ptr_or_index) {
        return_node = tree_node;
        tree_node = get_node(tree_node)->left;
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

namespace BIN_tree {
    template<typename ptr, typename function>
    static ptr tree_successor(ptr tree_node, ptr nil_ptr_or_index, function get_node) {
        if (get_node(tree_node)->right != nil_ptr_or_index) {
            return tree_minimum(get_node(tree_node)->right, nil_ptr_or_index, get_node);
        }
        ptr result_node = get_node(tree_node)->parent;
        while (result_node != nil_ptr_or_index && get_node(result_node)->right == tree_node) {
            tree_node = result_node;
            result_node = get_node(result_node)->parent;
        }
        return result_node;
    }

    template<typename ptr, typename function>
    ptr tree_predecessor(ptr tree_node, ptr nil_ptr_or_index, function get_node) {
        if (get_node(tree_node)->left != nil_ptr_or_index) {
            return tree_maximum(get_node(tree_node)->left);
        }
        ptr result_node = get_node(tree_node)->parent;
        while (result_node != nil_ptr_or_index && get_node(result_node)->left == tree_node) {
            tree_node = result_node;
            result_node = get_node(result_node)->parent;
        }
        return result_node;
    }


    template<typename ptr, typename function>
    static ptr replace_sub_tree(ptr dst_sub_tree_position, ptr src_sub_tree, ptr nil_ptr_or_index, function get_node) {
        get_node(src_sub_tree)->parent = get_node(dst_sub_tree_position)->parent;

        auto src_sub_tree_parent = get_node(src_sub_tree)->parent;
        if (src_sub_tree_parent == nil_ptr_or_index) {
        } else if (dst_sub_tree_position == get_node(src_sub_tree_parent)->left) {
            get_node(src_sub_tree_parent)->left = src_sub_tree;
        } else if (dst_sub_tree_position == get_node(src_sub_tree_parent)->right) {
            get_node(src_sub_tree_parent)->right = src_sub_tree;
        }
        return src_sub_tree;
    }


    template<typename ptr, typename function>
    ptr replace_sub_tree_left(ptr sub_tree, ptr new_left_sub_tree, ptr nil_ptr_or_index, function get_node) {
        get_node(sub_tree)->left = new_left_sub_tree;
        if (new_left_sub_tree != nil_ptr_or_index)
            get_node(new_left_sub_tree)->parent = sub_tree;
        return sub_tree;
    }


    template<typename ptr, typename function>
    ptr replace_sub_tree_right(ptr sub_tree, ptr new_right_sub_tree, ptr nil_ptr_or_index, function get_node) {
        get_node(sub_tree)->right = new_right_sub_tree;
        if (new_right_sub_tree != nil_ptr_or_index)
            get_node(new_right_sub_tree)->parent = sub_tree;
        return sub_tree;
    }

    template<typename ptr, typename function>
    static ptr clean_parent_to_current(ptr need_clean_sub_tree, ptr nil_ptr_or_index, function get_node) {
        auto temp_parent = get_node(need_clean_sub_tree)->parent;
        if (temp_parent == nil_ptr_or_index) {
        } else if (need_clean_sub_tree == get_node(temp_parent)->left) {
            get_node(temp_parent)->left = nil_ptr_or_index;
        } else if (need_clean_sub_tree == get_node(temp_parent)->right) {
            get_node(temp_parent)->right = nil_ptr_or_index;
        }
    }

    template<typename ptr, typename function>
    static ptr clean_current_to_father(ptr need_clean_sub_tree, ptr nil_ptr_or_index, function get_node) {
        get_node(need_clean_sub_tree)->parent = nil_ptr_or_index;
    }


    template<typename ptr, typename function>
    static ptr clean_sub_tree_father(ptr need_clean_sub_tree, ptr nil_ptr_or_index, function get_node) {
        auto temp_parent = get_node(need_clean_sub_tree)->parent;
        if (temp_parent == nil_ptr_or_index) {
        } else if (need_clean_sub_tree == get_node(temp_parent)->left) {
            get_node(temp_parent)->left = nil_ptr_or_index;
        } else if (need_clean_sub_tree == get_node(temp_parent)->right) {
            get_node(temp_parent)->right = nil_ptr_or_index;
        }
        get_node(need_clean_sub_tree)->parent = nil_ptr_or_index;
        return need_clean_sub_tree;
    }


    //            A                              B
    //         E     B                       A       D
    //            C    D                   E   C

    // 原本B是A的右子树，现在变成A是B的左子树
    template<typename ptr, typename function>
    static bool left_rotate(ptr node, ptr nil_ptr_or_index, function get_node) {
        if (node != nil_ptr_or_index && get_node(node)->right == nil_ptr_or_index) {
            return false;
        } else {
            auto A_node = node;
            auto B_node = get_node(node)->right;
            replace_sub_tree(A_node, B_node, nil_ptr_or_index, get_node);
            replace_sub_tree_right(A_node, get_node(B_node)->left, nil_ptr_or_index, get_node);
            replace_sub_tree_left(B_node, A_node, nil_ptr_or_index, get_node);
        }
    }

    //            A                 B
    //         B     E           C     A
    //      C    D                   D   E
    //
    // 原本B是A的左子树，现在变成A是B的右子树

    template<typename ptr, typename function>
    bool right_rotate(ptr node, ptr nil_ptr_or_index, function get_node) {
        if (node != nil_ptr_or_index && get_node(node)->left == nil_ptr_or_index) {
            return false;
        } else {
            auto A_node = node;
            auto B_node = get_node(node)->left;
            replace_sub_tree(A_node, B_node, nil_ptr_or_index, get_node);
            replace_sub_tree_left(A_node, get_node(B_node)->right, nil_ptr_or_index, get_node);
            replace_sub_tree_right(B_node, A_node, nil_ptr_or_index, get_node);
        }
    }


    template<typename ptr, typename function>
    ptr find_root(ptr node, ptr nil_ptr_or_index, function get_node) {
        if (node == nil_ptr_or_index) {
            return nil_ptr_or_index;
        } else {
            while (get_node(node)->parent != nil_ptr_or_index) {
                return find_root(get_node(node)->parent, nil_ptr_or_index, get_node);
            }
            return node;
        }
    }

    /**
     *
     * @tparam v_index
     * @tparam function
     * @param root
     * @param new_node_v_index
     * @param nil_ptr_or_index
     * @param get_node
     * @return 返回根指针
     */
    template<typename v_index, typename function>
    v_index add_new_node(v_index root, v_index new_node_v_index,
                         v_index nil_ptr_or_index,
                         function get_node) {
        auto insert_v_index_value =
                find_insert_position(root,
                                     new_node_v_index,
                                     nil_ptr_or_index,
                                     get_node);
        if (insert_v_index_value == nil_ptr_or_index) {
            return new_node_v_index;
        } else if (get_node(insert_v_index_value)->data < get_node(new_node_v_index)->data) {
            replace_sub_tree_right(insert_v_index_value, new_node_v_index, nil_ptr_or_index, get_node);
        } else {
            replace_sub_tree_left(insert_v_index_value, new_node_v_index, nil_ptr_or_index, get_node);
        }
        return root;
    }
}


template<typename RB_Tree_Node, typename function>
RB_Tree_Node left_rotate_with_color(RB_Tree_Node node,
                                    RB_Tree_Node nil_ptr_or_index,
                                    function get_node) {
    if (node != nil_ptr_or_index && get_node(node)->right == nil_ptr_or_index) {
        return nil_ptr_or_index;
    }

    auto temp = get_node(node)->right;
    auto temp_color = get_node(temp)->color;
    get_node(temp)->color = get_node(node)->color;
    get_node(node)->color = temp_color;
    BIN_tree::left_rotate(node, nil_ptr_or_index, get_node);
}


template<typename RB_Tree_Node, typename function>
RB_Tree_Node right_rotate_with_color(RB_Tree_Node node,
                                     RB_Tree_Node nil_ptr_or_index,
                                     function get_node) {
    if (node != nil_ptr_or_index && get_node(node)->left == nil_ptr_or_index) {
        return nil_ptr_or_index;
    }
    auto temp_color = get_node(get_node(node)->left)->color;
    get_node(get_node(node)->left)->color = get_node(node)->color;
    get_node(node)->color = temp_color;
    BIN_tree::right_rotate(node, nil_ptr_or_index, get_node);
}

namespace BIN_tree {
    /**
     * 只是将删除的节点从书中移除
     * @tparam v_index
     * @tparam function
     * @param root
     * @param delete_node
     * @param nil_ptr_or_index
     * @param get_node
     * @return 也是返回根指针
     */
    template<typename v_index, typename function>
    v_index delete_node_from_binary_search_tree(v_index root, v_index delete_node,
                                                v_index nil_ptr_or_index,
                                                function get_node) {
        if (delete_node == root &&
            get_node(delete_node)->left == nil_ptr_or_index &&
            get_node(delete_node)->right == nil_ptr_or_index) {
            return nil_ptr_or_index;
        } else if (delete_node == root && get_node(delete_node)->left != nil_ptr_or_index &&
                   get_node(delete_node)->right == nil_ptr_or_index) {
            return get_node(delete_node)->left;
        } else if (delete_node == root && get_node(delete_node)->left == nil_ptr_or_index &&
                   get_node(delete_node)->right != nil_ptr_or_index) {
            return get_node(delete_node)->right;
        }
        if (get_node(delete_node)->right == nil_ptr_or_index && get_node(delete_node)->left == nil_ptr_or_index) {
            // 如果被删除的是叶子结点，那么就清除父结点的索引
            clean_sub_tree_father(delete_node, nil_ptr_or_index, get_node);
        } else if (get_node(delete_node)->right == nil_ptr_or_index && get_node(delete_node)->left !=
                   nil_ptr_or_index) {
            // 右子树为空
            replace_sub_tree(delete_node, get_node(delete_node)->left, nil_ptr_or_index, get_node);
        } else if (get_node(delete_node)->right != nil_ptr_or_index && get_node(delete_node)->left ==
                   nil_ptr_or_index) {
            // 左子树为空
            replace_sub_tree(delete_node, get_node(delete_node)->right, nil_ptr_or_index, get_node);
        } else if (get_node(delete_node)->right != nil_ptr_or_index && get_node(delete_node)->left !=
                   nil_ptr_or_index) {
            // 寻找后继
            auto successor = tree_successor(delete_node,
                                            nil_ptr_or_index, get_node);
            // 这行还是有问题的，还是编译不过，
            if (get_node(successor)->left == nil_ptr_or_index && get_node(successor)->right == nil_ptr_or_index) {
                clean_sub_tree_father(successor, nil_ptr_or_index, get_node);
            } else if (get_node(successor)->left == nil_ptr_or_index && get_node(successor)->right !=
                       nil_ptr_or_index) {
                replace_sub_tree(successor, get_node(successor)->right, nil_ptr_or_index, get_node);
            }
            replace_sub_tree(delete_node, successor, nil_ptr_or_index, get_node);

            if (get_node(delete_node)->parent == nil_ptr_or_index) {
                root = successor;
            }
            replace_sub_tree_left(successor, get_node(delete_node)->left, nil_ptr_or_index, get_node);
            replace_sub_tree_right(successor, get_node(delete_node)->right, nil_ptr_or_index, get_node);
        }
        return root;
    }
}


template<typename value_type, typename ptr>
ptr tree_find_value(ptr root, value_type data) {
    return BIN_tree::tree_find_value(root, data, static_cast<ptr>(nullptr),
                                     [](ptr insert_node) { return insert_node; });
}

template<typename T>
T find_insert_position(T root, T new_node) {
    return find_insert_position(root, new_node,
                                static_cast<T>(nullptr),
                                [](T insert_node) { return insert_node; });
}

template<typename T>
std::vector<T> &find_all_leaf_node(T node, std::vector<T> &result) {
    return find_all_leaf_node(node, result, static_cast<T>(nullptr), [](T insert_node) { return insert_node; });
}
#endif //TREE_FUNCTION_H
