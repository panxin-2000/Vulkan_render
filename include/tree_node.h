//
// Created by 潘鑫 on 2025/9/24.
//

#ifndef TREE_NODE_H
#define TREE_NODE_H
#include "add_char_calculata.h"
#include "add_char_calculata.h"

template<class T>
class Tree_Node {
public:
    T *parent;
    T *left;
    T *right;
};

template<class T>
class RB_Tree_Node : public Tree_Node<RB_Tree_Node<T> > {
public:
    RB_Tree_Node() {
    }

    ~RB_Tree_Node(void) {
    }

    T data;
    int color_tag;

public:
    struct RB_Tree_Node<T> *find_miximum_leaf(struct RB_Tree_Node *root) {
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

    bool insert_node_to_binary_search_tree(RB_Tree_Node *root, RB_Tree_Node &new_node) {
        RB_Tree_Node *new_root = root;
        RB_Tree_Node *insert_node = nullptr;
        while (new_root != nullptr) {
            insert_node = new_root;
            if (insert_node->data < new_node.data) {
                new_root = new_root->right;
            } else {
                insert_node = new_root->left;
            }
        }
        if (insert_node->data < new_node.data) {
            insert_node->right = new_node;
            new_node.parent = insert_node;
        } else {
            insert_node->left = new_node;
            new_node.parent = insert_node;
        }
        // 插入完成之后，然后再重新进行排序，根据左右树的高度，看看是否需要调节高度
    }


    /**
     * 如果返回为 true 的时候，可以将插入的新结点，之后再查到这个树的根结点
     * @param root
     * @param new_node
     * @return
     */
    bool insert_node_to_binary_search_tree(RB_Tree_Node &root, RB_Tree_Node &new_node) {
        // 这里是使用递归的方式来
        if (root.data < new_node.data && root.right == nullptr) {
            root.right = new_node;
            new_node.parent = root;
        } else if (root.data < new_node.data && root.right != nullptr) {
            insert_node_to_binary_search_tree(root.right, new_node);
        } else if (root.data > new_node.data && root.left == nullptr) {
            root.left = new_node;
            new_node.parent = root;
        } else if (root.data > new_node.data && root.left != nullptr) {
            insert_node_to_binary_search_tree(root.right, new_node);
        }
        while (root) {
        }
        //需要插入一个值，那就需要按照T data 的值进行排序
        // 如果当前的值大，就去找当前节点的右结点，
        // 如果当前的值小，那就找当前节点的左结点，
        // 如果一个左或者右为空的时候，那么就去将这个结点插入

        // 插入完成之后，然后再重新进行排序，根据左右树的高度，看看是否需要调节高度
    }

    bool delete_node_from_binary_search_tree(RB_Tree_Node *root, RB_Tree_Node &delete_node) {
        if (delete_node.parent == nullptr) {
            // 删除的是根结点，那么根结点为空
        }
        if (delete_node.right == nullptr && delete_node.left == nullptr) {
            // 如果被删除的是叶子结点，那么就清除父结点的索引
            if (delete_node.parent->left == delete_node) {
                delete_node.parent->left = nullptr;
            } else if (delete_node.parent->right == delete_node) {
                delete_node.parent->right = nullptr;
            }
        } else if (delete_node.right == nullptr && delete_node.left != nullptr) {
            // 右子树为空
            if (delete_node.parent->left == delete_node) {
                // 为父亲的左子树
                delete_node.parent->left = delete_node.left; // 将父亲的左子树更新为删除节点的左子树
                delete_node.left->parent = delete_node.parent; // 将删除结点的左子树的父亲更新为删除结点的父亲
            } else if (delete_node.parent->right == delete_node) {
                // 就不继续注释了
                delete_node.parent->right = delete_node.left;
                delete_node.left->parent = delete_node.parent;
            }
        } else if (delete_node.right != nullptr && delete_node.left == nullptr) {
            // 左子树为空
            if (delete_node.parent->left == delete_node) {
                delete_node.parent->left = delete_node.right;
                delete_node.right->parent = delete_node.parent;
            } else if (delete_node.parent->right == delete_node) {
                delete_node.parent->right = delete_node.right;
                delete_node.right->parent = delete_node.parent;
            }
        } else if (delete_node.right != nullptr && delete_node.left != nullptr) {
            // 寻找后继
            auto successor = tree_successor(delete_node);
            if (successor->left == nullptr && successor->right == nullptr) {
                if (successor->parent->left == successor) {
                    successor->parent->left = nullptr;
                } else if (successor->parent->right == successor) {
                    successor->parent->right = nullptr;
                }
            } else if (successor->left == nullptr && successor->right != nullptr) {
                if (successor->parent == delete_node) {
                    // 后继是删除结点的右孩子， // 想办法跳过就可以了
                    delete_node.right = successor->right;
                } else {
                    // 后继不是删除结点的右孩子
                    successor->parent->left = successor->right;
                    successor->right->parent = successor->parent;
                }
                // successor 这个时候是算法一个最小的叶子结点了
                successor->parent = delete_node.parent;
                if (delete_node.parent->left == delete_node) {
                    delete_node.parent->left = successor;
                } else if (delete_node.parent->right == delete_node) {
                    delete_node.parent->right = successor;
                }
                // 清理后继和其父亲的关系
                successor->left = delete_node.left;
                successor->left->parent = successor;

                successor->right = delete_node.right;
                successor->right->parent = successor;
                // 将后继与被删除的结点进行替换
            }
        }
        // 找到需要删除的后继，之后再做相应的操作，


        // 判断后继的是左右子结点都是空，然后只有左结点为空，右结点不为空
        // 如果左右结点为空，那么直接将这个后继添加到这里好，
        // 如果右结点不为空呢？之后应该怎么解决呢？
        // 将后继的右子树替代掉后继的位置，然后将后继替代掉删除的位置

        // 插入完成之后，然后再重新进行排序，根据左右树的高度，看看是否需要调节高度
        // 高度重排是红黑特有的还是二叉平衡树也有的？
    }

    struct RB_Tree_Node *find_root(struct RB_Tree_Node *node) {
        if (node == nullptr) {
            return nullptr;
        } else {
            while (node->parent != nullptr) {
                return find_root(node->parent);
            }
            return node;
        }
    }

    int tree_add_after_node(struct RB_Tree_Node *root, struct RB_Tree_Node *new_node) {
        if (root == nullptr) {
            root = new_node; // 这里是添加首个数字的位置
        } else {
            struct RB_Tree_Node *new_root = root;
            struct RB_Tree_Node *miximum_leaf = new_node->find_miximum_leaf(new_root);
            while (miximum_leaf->parent->right != nullptr) {
                miximum_leaf = miximum_leaf->parent;
            }
            add_new_node_to_right(miximum_leaf->parent, new_node);
        }
    }

    int add_node_to_left_child(struct RB_Tree_Node *left) {
        this->left = left;
        left->parent = this;
    }

    int add_node_to_right_child(struct RB_Tree_Node *right) {
        this->right = right;
        right->parent = this;
    }

    int add_new_node_before_parent(struct RB_Tree_Node *node, struct RB_Tree_Node *new_node) {
        new_node->parent = node->parent;
        new_node->left = node;
        node->parent = new_node;
        if (new_node->parent != nullptr && new_node->parent->left == node) {
            new_node->parent->left = new_node;
        } else if (new_node->parent != nullptr && new_node->parent->right == node) {
            new_node->parent->right = new_node;
        }
    }

    int add_new_node_to_right(struct RB_Tree_Node *node, struct RB_Tree_Node *new_node) {
        new_node->right = node->right; // 新节点的右孩子更新为将要原本的右孩子
        node->right = new_node; // 更新node的右孩子
        new_node->parent = node; // 更新新节点的parent
        if (new_node->right != nullptr) {
            new_node->right->parent = new_node; // 如果原本的右孩子不为空，更新原本的父节点
        }
    }
};


#endif //TREE_NODE_H
