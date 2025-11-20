//
// Created by 潘鑫 on 2025/11/21.
//

#ifndef RB_TREE_FUNCTION_H
#define RB_TREE_FUNCTION_H
#include "tree_function.h"

enum RB_Tree_Node_color {
    RB_Tree_BLACK = 0,
    RB_Tree_RED = 1,
};

namespace BIN_tree {
    template<typename RB_Tree_Node, typename function>
    bool RB_insert_fix(RB_Tree_Node root, RB_Tree_Node new_node,
                       RB_Tree_Node nil_ptr_or_index,
                       function get_node) {
        // 问题是什么？nil还是需要清理的？什么时候清理呢？

        auto new_node_parent = get_node(new_node)->parent;
        auto new_node_grandparent = get_node(new_node_parent)->parent;

        while (get_node(new_node)->color == RB_Tree_RED &&
               new_node_parent != nil_ptr_or_index &&
               new_node_grandparent != nil_ptr_or_index &&
               get_node(new_node_parent)->color == RB_Tree_RED) {
            if (new_node_parent == get_node(new_node_grandparent)->left) {
                auto uncle = get_node(new_node_grandparent)->right;
                if (uncle != nil_ptr_or_index && get_node(uncle)->color == RB_Tree_RED) {
                    get_node(uncle)->color = RB_Tree_BLACK;
                    get_node(new_node_grandparent)->color = RB_Tree_RED;
                    get_node(new_node_parent)->color = RB_Tree_BLACK;
                    new_node = new_node_grandparent;
                } else if ((uncle == nil_ptr_or_index || get_node(uncle)->color == RB_Tree_BLACK) &&
                           new_node == get_node(new_node_parent)->right) {
                    new_node = new_node_parent;
                    get_node(new_node)->left_rotate(new_node);
                } else if ((uncle == nil_ptr_or_index || get_node(uncle)->color == RB_Tree_BLACK) &&
                           new_node == get_node(new_node_parent)->left) {
                    get_node(new_node_parent)->color = RB_Tree_BLACK;
                    get_node(new_node_grandparent)->color = RB_Tree_RED;
                    right_rotate(new_node_grandparent, nil_ptr_or_index, get_node);
                }
            } else {
                // 将上面的左右给翻转一次
                auto uncle = get_node(new_node_grandparent)->left;
                // uncle 为 null 的情况也是需要考虑的
                if (uncle != nil_ptr_or_index && get_node(uncle)->color == RB_Tree_RED) {
                    get_node(uncle)->color = RB_Tree_BLACK;
                    get_node(new_node_grandparent)->color = RB_Tree_RED;
                    get_node(new_node_parent)->color = RB_Tree_BLACK;
                    new_node = new_node_grandparent;
                } else if ((uncle == nil_ptr_or_index || get_node(uncle)->color == RB_Tree_BLACK) &&
                           new_node == (get_node(new_node)->parent)->left) {
                    new_node = new_node_parent;
                    right_rotate(new_node, nil_ptr_or_index, get_node); // 这里的旋转似乎也应该换一个位置
                } else if ((uncle == nil_ptr_or_index || get_node(uncle)->color == RB_Tree_BLACK) &&
                           new_node == get_node(get_node(new_node)->parent)->right) {
                    get_node(new_node_parent)->color = RB_Tree_BLACK;
                    get_node(new_node_grandparent)->color = RB_Tree_RED;
                    left_rotate(new_node_grandparent);
                }
            }
        }
        if (get_node(new_node)->color == RB_Tree_RED && new_node_parent == nil_ptr_or_index) {
            get_node(new_node)->color = RB_Tree_BLACK;
        }
    }

    template<typename v_index, typename function>
    v_index insert_node_to_RB_search_tree(v_index root, v_index new_node_v_index,
                                          v_index nil_ptr_or_index,
                                          function get_node) {
        get_node(new_node_v_index)->color = RB_Tree_RED;
        auto insert_v_index_value =
                find_insert_position(root,
                                     new_node_v_index,
                                     nil_ptr_or_index,
                                     get_node);
        if (insert_v_index_value == nil_ptr_or_index) {
            get_node(new_node_v_index)->color = RB_Tree_BLACK;
            return new_node_v_index;
        } else if (get_node(insert_v_index_value)->data < get_node(new_node_v_index)->data) {
            replace_sub_tree_right(insert_v_index_value, new_node_v_index, nil_ptr_or_index, get_node);
        } else {
            replace_sub_tree_left(insert_v_index_value, new_node_v_index, nil_ptr_or_index, get_node);
        }
        if (get_node(insert_v_index_value)->color == RB_Tree_RED) {
            RB_insert_fix(root, new_node_v_index, nil_ptr_or_index, get_node);
        }
        return find_root(root, nil_ptr_or_index, get_node);
    }


    /**
     *
     * @param root
     * @param delete_node
     * @return 返回根结点
     */
    template<typename RB_Tree_Node, typename function>
    RB_Tree_Node delete_node_from_RB_tree(RB_Tree_Node root, RB_Tree_Node delete_node,
                                          RB_Tree_Node nil_ptr_or_index,
                                          function get_node) {
        if (delete_node == nil_ptr_or_index) {
            return root;
        }
        auto delete_node_color = get_node(delete_node)->color;
        auto need_fix_node = root; {
            // 删除的是根结点，左子树为空，或者右子树为空，那么不为空的只有一个红色的结点
            if (delete_node == root &&
                get_node(delete_node)->left == nil_ptr_or_index &&
                get_node(delete_node)->right == nil_ptr_or_index) {
                return nil_ptr_or_index;
            } else if (delete_node == root &&
                       get_node(delete_node)->left != nil_ptr_or_index &&
                       get_node(delete_node)->right == nil_ptr_or_index) {
                get_node(get_node(delete_node)->left)->color = RB_Tree_BLACK;
                return get_node(delete_node)->left;
            } else if (delete_node == root &&
                       get_node(delete_node)->left == nil_ptr_or_index &&
                       get_node(delete_node)->right != nil_ptr_or_index) {
                get_node(get_node(delete_node)->right)->color = RB_Tree_BLACK;
                return get_node(delete_node)->right;
            }
        }

        if (get_node(delete_node)->right == nil_ptr_or_index &&
            get_node(delete_node)->left == nil_ptr_or_index) {
            // 如果被删除的是叶子结点，那么就清除父结点的索引
            // 只有叶子结点这里需要一个额外的处理
            // clean_parent_to_current(&delete_node);
            RB_delete_fix(root, delete_node); //
            clean_sub_tree_father(delete_node); // 这个函数里面多了一步，导致了一个小问题
            return root;
        } else if (get_node(delete_node)->right == nil_ptr_or_index &&
                   get_node(delete_node)->left != nil_ptr_or_index) {
            // 右子树为空
            need_fix_node = get_node(delete_node)->left;
            replace_sub_tree(delete_node, get_node(delete_node)->left, nil_ptr_or_index, get_node);
        } else if (get_node(delete_node)->right != nil_ptr_or_index &&
                   get_node(delete_node)->left == nil_ptr_or_index) {
            // 左子树为空
            need_fix_node = get_node(delete_node)->right;
            replace_sub_tree(delete_node, get_node(delete_node)->right, nil_ptr_or_index, get_node);
        } else if (get_node(delete_node)->right != nil_ptr_or_index &&
                   get_node(delete_node)->left != nil_ptr_or_index) {
            // 寻找后继
            auto successor = tree_successor(delete_node); // 这行还是有问题的，还是编译不过，
            delete_node_color = get_node(successor)->color;
            if (get_node(successor)->left == nil_ptr_or_index &&
                get_node(successor)->right == nil_ptr_or_index) {
                need_fix_node = successor;
                // clean_parent_to_current(successor);
                if (delete_node_color == RB_Tree_BLACK) {
                    RB_delete_fix(root, need_fix_node);
                }
                delete_node_color = RB_Tree_RED;
                clean_sub_tree_father(successor);
                // successor是一个叶子结点，是红是黑是无所谓的
            } else if (get_node(successor)->left == nil_ptr_or_index &&
                       get_node(successor)->right != nil_ptr_or_index) {
                // 后继右子树替换掉后继原本的位置
                // 后继是删除结点的右孩子 时也是执行这个操作
                need_fix_node = get_node(successor)->right;
                replace_sub_tree(successor, get_node(successor)->right, nil_ptr_or_index, get_node);
            }
            replace_sub_tree(delete_node, successor, nil_ptr_or_index, get_node);

            if (get_node(delete_node)->parent == nil_ptr_or_index) {
                root = successor;
                need_fix_node = root;
            }
            replace_sub_tree_left(successor, get_node(delete_node)->left, nil_ptr_or_index, get_node);
            replace_sub_tree_right(successor, get_node(delete_node)->right, nil_ptr_or_index, get_node);
            get_node(successor)->color = get_node(delete_node)->color;
        }
        if (delete_node_color == RB_Tree_BLACK) {
            RB_delete_fix(root, need_fix_node);
        }
        return root;
    }

    template<typename RB_Tree_Node, typename function>
    bool RB_delete_fix(RB_Tree_Node root, RB_Tree_Node need_fix_node,
                       RB_Tree_Node nil_ptr_or_index,
                       function get_node) {
        while (need_fix_node != root && get_node(need_fix_node)->color == RB_Tree_BLACK) {
            auto need_fix_node_parent = get_node(need_fix_node)->parent;
            if (need_fix_node == get_node(need_fix_node_parent)->left) {
                auto w_node = get_node(need_fix_node_parent)->right;
                if (get_node(w_node)->color == RB_Tree_RED) {
                    left_rotate_with_color(need_fix_node_parent, nil_ptr_or_index, get_node);
                    w_node = get_node(need_fix_node_parent)->right;
                }
                if ((get_node(w_node)->left == nil_ptr_or_index ||
                     get_node(get_node(w_node)->left)->color == RB_Tree_BLACK)
                    && (get_node(w_node)->right == nil_ptr_or_index ||
                        get_node(get_node(w_node)->right)->color ==
                        RB_Tree_BLACK)) {
                    get_node(w_node)->color = RB_Tree_RED;
                    need_fix_node = need_fix_node_parent;
                }
                if ((get_node(w_node)->left != nil_ptr_or_index &&
                     get_node(get_node(w_node)->left)->color == RB_Tree_RED)
                    && (get_node(w_node)->right == nil_ptr_or_index ||
                        get_node(get_node(w_node)->right)->color == RB_Tree_BLACK)) {
                    right_rotate_with_color(w_node, nil_ptr_or_index, get_node);
                    w_node = get_node(need_fix_node_parent)->right;
                }
                if (get_node(w_node)->right != nil_ptr_or_index &&
                    get_node(get_node(w_node)->right)->color == RB_Tree_RED) {
                    get_node(need_fix_node_parent)->color = RB_Tree_BLACK;
                    left_rotate(need_fix_node_parent);
                    get_node(w_node)->right->color = RB_Tree_BLACK;
                    need_fix_node = root;
                }
            } else {
                auto w_node = get_node(need_fix_node_parent)->left;
                if (get_node(w_node)->color == RB_Tree_RED) {
                    right_rotate_with_color(need_fix_node_parent, nil_ptr_or_index, get_node);
                    w_node = get_node(need_fix_node_parent)->left;
                }
                if ((get_node(w_node)->left == nil_ptr_or_index ||
                     get_node(get_node(w_node)->left)->color == RB_Tree_BLACK)
                    && (get_node(w_node)->right == nil_ptr_or_index ||
                        get_node(get_node(w_node)->right)->color == RB_Tree_BLACK)) {
                    get_node(w_node)->color = RB_Tree_RED;
                    need_fix_node = need_fix_node_parent;
                }
                if ((get_node(w_node)->right != nil_ptr_or_index &&
                     get_node(get_node(w_node)->right)->color == RB_Tree_RED)
                    && (get_node(w_node)->left == nil_ptr_or_index ||
                        get_node(get_node(w_node)->left)->color == RB_Tree_BLACK)) {
                    left_rotate_with_color(w_node, nil_ptr_or_index, get_node);
                    w_node = get_node(need_fix_node_parent)->left;
                }
                if (get_node(w_node)->left != nil_ptr_or_index &&
                    get_node(get_node(w_node)->left)->color == RB_Tree_RED) {
                    get_node(need_fix_node_parent)->color = RB_Tree_BLACK;
                    right_rotate_with_color(need_fix_node_parent, nil_ptr_or_index, get_node);
                    get_node(get_node(w_node)->left)->color = RB_Tree_BLACK;
                    need_fix_node = root;
                }
            }
        }
        need_fix_node->color = RB_Tree_BLACK;
    }
}

#endif //RB_TREE_FUNCTION_H
