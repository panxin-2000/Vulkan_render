//
// Created by 潘鑫 on 2025/10/6.
//

#ifndef RB_TREE_NODE_H
#define RB_TREE_NODE_H
#include "binary_Tree_Node.h"


// 不能再去继承binary_Tree_Node   ，只能从Tree_Node继承
template<class T>
class RB_Tree_Node : public Tree_Node<RB_Tree_Node<T> > {
    // 这里有一个使用模版不断套娃的小技巧
public:
    RB_Tree_Node() {
    }

    ~RB_Tree_Node() {
    }

    T data;


    // 持久树，持久树的目的是为了保存两个操作 persistent
    // 其实有一个问题，是否可以只用保存一个新的树到叶子结点的链而保存保存整个树中的内容呢？
    // 红黑树不行，因为不止一个从叶子到根的指针或者颜色改变，至少需要改半个树吧？或者说至少需要十个树的结点
    // 因为有时的黑高也是一个判断的选项，所以有时候需要更改半个树来重新维持持久的操作

    enum RB_Tree_Node_color {
        RB_Tree_BLACK = 0,
        RB_Tree_RED = 1,
    };

    RB_Tree_Node_color color;


    RB_Tree_Node *tree_insert_value(RB_Tree_Node *root, T data) {
        RB_Tree_Node &nodes = *new RB_Tree_Node<T>;
        nodes.right = nullptr;
        nodes.left = nullptr;
        nodes.parent = nullptr;
        nodes.data = data;
        return root->insert_node_to_binary_search_tree(root, &nodes);
    }

    std::vector<T> *translate(std::vector<RB_Tree_Node *> src_s) {
        auto result = new std::vector<T>();
        for (auto src: src_s) {
            result->push_back(src->data);
        }
        return result;
    }


    /**
     * 看起来是能用的，实际上差了一个很重要的内容，哨兵
     * @param root
     * @param new_node
     * @return
     */

    bool RB_insert_fix(RB_Tree_Node *root, RB_Tree_Node *new_node) {
        // 问题是什么？nil还是需要清理的？什么时候清理呢？

        while (new_node->color == RB_Tree_RED && new_node->parent != nullptr &&
               new_node->parent->parent != nullptr && new_node->parent->color == RB_Tree_RED) {
            auto grandfather = new_node->parent->parent;
            if (new_node->parent == grandfather->left) {
                auto uncle = grandfather->right;
                if (uncle != nullptr && uncle->color == RB_Tree_RED) {
                    uncle->color = RB_Tree_BLACK;
                    grandfather->color = RB_Tree_RED;
                    new_node->parent->color = RB_Tree_BLACK;
                    new_node = grandfather;
                } else if ((uncle == nullptr || uncle->color == RB_Tree_BLACK) && new_node == new_node->parent->right) {
                    new_node = new_node->parent;
                    new_node->left_rotate(new_node);
                } else if ((uncle == nullptr || uncle->color == RB_Tree_BLACK) && new_node == new_node->parent->left) {
                    new_node->parent->color = RB_Tree_BLACK;
                    grandfather->color = RB_Tree_RED;
                    new_node->right_rotate(grandfather);
                }
            } else {
                // 将上面的左右给翻转一次
                auto uncle = grandfather->left;
                // uncle 为 null 的情况也是需要考虑的
                if (uncle != nullptr && uncle->color == RB_Tree_RED) {
                    uncle->color = RB_Tree_BLACK;
                    grandfather->color = RB_Tree_RED;
                    new_node->parent->color = RB_Tree_BLACK;
                    new_node = grandfather;
                } else if ((uncle == nullptr || uncle->color == RB_Tree_BLACK) && new_node == new_node->parent->left) {
                    new_node = new_node->parent;
                    new_node->right_rotate(new_node); // 这里的旋转似乎也应该换一个位置
                } else if ((uncle == nullptr || uncle->color == RB_Tree_BLACK) && new_node == new_node->parent->right) {
                    new_node->parent->color = RB_Tree_BLACK;
                    grandfather->color = RB_Tree_RED;
                    new_node->left_rotate(grandfather);
                }
            }
        }
        if (new_node->color == RB_Tree_RED && new_node->parent == nullptr) {
            new_node->color = RB_Tree_BLACK;
        }
    }

    RB_Tree_Node *insert_node_to_binary_search_tree(RB_Tree_Node *root, RB_Tree_Node *new_node) {
        // 其实稍微有点不想做，为什么呢？因为感觉有点细碎
        // 但是还是需要去做的，其实重点还是去写测试的规则
        // 这里其实是有五条规则的，但是我已经并不能
        // 比如一个红色结点下的两个结点的颜色比如是黑色的
        // 树的根结点是黑色的
        // 树的黑高，
        // 先不考虑删除的事情，刚好给了一个机会把插入实现了
        // 先把全部的代码复制过来。
        new_node->color = RB_Tree_RED;
        RB_Tree_Node *new_root = root;
        RB_Tree_Node *insert_node = nullptr;
        while (new_root != nullptr) {
            insert_node = new_root;
            if (insert_node->data < new_node->data) {
                new_root = new_root->right;
            } else {
                new_root = new_root->left;
            }
        }
        if (insert_node == nullptr) {
            // 这里是直接插入根结点，根结点的颜色应该是黑色
            new_node->color = RB_Tree_BLACK;
            return new_node;
        } else if (insert_node->data < new_node->data) {
            RB_Tree_Node::replace_sub_tree_right(insert_node, new_node);
        } else {
            RB_Tree_Node::replace_sub_tree_left(insert_node, new_node);
        }
        if (insert_node->color == RB_Tree_RED) {
            // 那么这里就是两个红色结点了，是需要调整的，之后的情况，我已经不能纯粹的记住了
            RB_insert_fix(root, new_node);
        }
        return root->find_root(root); // 这里的问题，返回的估计有问题，在去查找一遍根结点，之后再返回吧
    }

    RB_Tree_Node *left_rotate_with_color(RB_Tree_Node *node) {
        if (node != nullptr && node->right == nullptr) {
            return nullptr;
        }
        auto temp_color = node->right->color;
        node->right->color = node->color;
        node->color = temp_color;
        RB_Tree_Node::left_rotate(node);
    }

    RB_Tree_Node *right_rotate_with_color(RB_Tree_Node *node) {
        if (node != nullptr && node->left == nullptr) {
            return nullptr;
        }
        auto temp_color = node->left->color;
        node->left->color = node->color;
        node->color = temp_color;
        RB_Tree_Node::right_rotate(node);
    }


    /**
     *
     * @param root
     * @param delete_node
     * @return 返回根结点
     */
    RB_Tree_Node *delete_node_from_binary_search_tree(RB_Tree_Node *root, RB_Tree_Node *delete_node) {
        if (delete_node == nullptr) {
            return root;
        }
        auto delete_node_color = delete_node->color;
        auto need_fix_node = root; {
            // 删除的是根结点，左子树为空，或者右子树为空，那么不为空的只有一个红色的结点
            if (delete_node == root && delete_node->left == nullptr && delete_node->right == nullptr) {
                return nullptr;
            } else if (delete_node == root && delete_node->left != nullptr && delete_node->right == nullptr) {
                delete_node->left->color = RB_Tree_BLACK;
                return delete_node->left;
            } else if (delete_node == root && delete_node->left == nullptr && delete_node->right != nullptr) {
                delete_node->right->color = RB_Tree_BLACK;
                return delete_node->right;
            }
        }

        if (delete_node->right == nullptr && delete_node->left == nullptr) {
            // 如果被删除的是叶子结点，那么就清除父结点的索引
            // 只有叶子结点这里需要一个额外的处理
            // RB_Tree_Node::clean_parent_to_current(&delete_node);
            RB_delete_fix(root, delete_node); //
            RB_Tree_Node::clean_sub_tree_father(delete_node); // 这个函数里面多了一步，导致了一个小问题
            return root;
        } else if (delete_node->right == nullptr && delete_node->left != nullptr) {
            // 右子树为空
            need_fix_node = delete_node->left;
            delete_node->replace_sub_tree(delete_node, delete_node->left);
        } else if (delete_node->right != nullptr && delete_node->left == nullptr) {
            // 左子树为空
            need_fix_node = delete_node->right;
            delete_node->replace_sub_tree(delete_node, delete_node->right);
        } else if (delete_node->right != nullptr && delete_node->left != nullptr) {
            // 寻找后继
            auto successor = root->tree_successor(delete_node); // 这行还是有问题的，还是编译不过，
            delete_node_color = successor->color;
            if (successor->left == nullptr && successor->right == nullptr) {
                need_fix_node = successor;
                // RB_Tree_Node::clean_parent_to_current(successor);
                if (delete_node_color == RB_Tree_BLACK) {
                    RB_delete_fix(root, need_fix_node);
                }
                delete_node_color = RB_Tree_RED;
                RB_Tree_Node::clean_sub_tree_father(successor);
                // successor是一个叶子结点，是红是黑是无所谓的
            } else if (successor->left == nullptr && successor->right != nullptr) {
                // 后继右子树替换掉后继原本的位置
                // 后继是删除结点的右孩子 时也是执行这个操作
                need_fix_node = successor->right;
                successor->replace_sub_tree(successor, successor->right);
            }
            successor->replace_sub_tree(delete_node, successor);

            if (delete_node->parent == nullptr) {
                root = successor;
                need_fix_node = root;
            }
            successor->replace_sub_tree_left(successor, delete_node->left);
            successor->replace_sub_tree_right(successor, delete_node->right);
            successor->color = delete_node->color;
        }
        if (delete_node_color == RB_Tree_BLACK) {
            RB_delete_fix(root, need_fix_node);
        }
        return root;
    }

    bool RB_delete_fix(RB_Tree_Node *root, RB_Tree_Node *need_fix_node) {
        while (need_fix_node != root && need_fix_node->color == RB_Tree_BLACK) {
            if (need_fix_node == need_fix_node->parent->left) {
                auto w_node = need_fix_node->parent->right;
                if (w_node->color == RB_Tree_RED) {
                    RB_Tree_Node::left_rotate_with_color(need_fix_node->parent);
                    w_node = need_fix_node->parent->right;
                }
                if ((w_node->left == nullptr || w_node->left->color == RB_Tree_BLACK) &&
                    (w_node->right == nullptr || w_node->right->color == RB_Tree_BLACK)) {
                    w_node->color = RB_Tree_RED;
                    need_fix_node = need_fix_node->parent;
                }
                if ((w_node->left != nullptr && w_node->left->color == RB_Tree_RED) &&
                    (w_node->right == nullptr || w_node->right->color == RB_Tree_BLACK)) {
                    RB_Tree_Node::right_rotate_with_color(w_node);
                    w_node = need_fix_node->parent->right;
                }
                if (w_node->right != nullptr && w_node->right->color == RB_Tree_RED) {
                    need_fix_node->parent->color = RB_Tree_BLACK;
                    RB_Tree_Node::left_rotate(need_fix_node->parent);
                    w_node->right->color = RB_Tree_BLACK;
                    need_fix_node = root;
                }
            } else {
                auto w_node = need_fix_node->parent->left;
                if (w_node->color == RB_Tree_RED) {
                    RB_Tree_Node::right_rotate_with_color(need_fix_node->parent);
                    w_node = need_fix_node->parent->left;
                }
                if ((w_node->left == nullptr || w_node->left->color == RB_Tree_BLACK) &&
                    (w_node->right == nullptr || w_node->right->color == RB_Tree_BLACK)) {
                    w_node->color = RB_Tree_RED;
                    need_fix_node = need_fix_node->parent;
                }
                if ((w_node->right != nullptr && w_node->right->color == RB_Tree_RED) &&
                    (w_node->left == nullptr || w_node->left->color == RB_Tree_BLACK)) {
                    RB_Tree_Node::left_rotate_with_color(w_node);
                    w_node = need_fix_node->parent->left;
                }
                if (w_node->left != nullptr && w_node->left->color == RB_Tree_RED) {
                    need_fix_node->parent->color = RB_Tree_BLACK;
                    RB_Tree_Node::right_rotate_with_color(need_fix_node->parent);
                    w_node->left->color = RB_Tree_BLACK;
                    need_fix_node = root;
                }
            }
        }
        need_fix_node->color = RB_Tree_BLACK;
    }
};


#endif //RB_TREE_NODE_H
