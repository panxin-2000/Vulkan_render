//
// Created by 潘鑫 on 2025/10/6.
//

#ifndef BALANCE_TREE_NODE_H
#define BALANCE_TREE_NODE_H
#include "tree_node.h"

template<class T>
class binary_Tree_Node : public Tree_Node<binary_Tree_Node<T> > {
public:
    binary_Tree_Node() {
    }

    ~binary_Tree_Node(void) {
    }

    T data;

public:
    // 持久树，持久树的目的是为了保存两个操作 persistent
    // 其实有一个问题，是否可以只用保存一个新的树到叶子结点的链而保存保存整个树中的内容呢？
    // 这里是没有问题的，从那个结点向上到根之间是需要保存的
    //
    binary_Tree_Node *tree_insert_value(binary_Tree_Node *root, T data) {
        const auto nodes = new binary_Tree_Node<T>;
        nodes->right = nullptr;
        nodes->left = nullptr;
        nodes->parent = nullptr;
        nodes->data = data;
        return root->insert_node_to_binary_search_tree(root, nodes);
    }

    binary_Tree_Node *tree_insert_value_with_history(binary_Tree_Node *root, T data) {
        const auto nodes = new binary_Tree_Node<T>;
        nodes->right = nullptr;
        nodes->left = nullptr;
        nodes->parent = nullptr;
        nodes->data = data;
        return root->insert_node_to_binary_search_tree_with_history(root, nodes);
    }


    /**
     * 确定一下返回值，返回值总是返回树的根
     * @param root
     * @param new_node
     * @return
     */
    binary_Tree_Node *insert_node_to_binary_search_tree(binary_Tree_Node *root, binary_Tree_Node *new_node) {
        if (new_node == nullptr) {
            return root;
        }
        auto insert_node = find_insert_position(root, new_node);

        if (insert_node == nullptr) {
            return new_node;
        } else if (insert_node->data < new_node->data) {
            binary_Tree_Node::replace_sub_tree_right(insert_node, new_node);
        } else {
            binary_Tree_Node::replace_sub_tree_left(insert_node, new_node);
        }
        return root;
        // 插入完成之后，然后再重新进行排序，根据左右树的高度，看看是否需要调节高度
    }

    binary_Tree_Node *
    insert_node_to_binary_search_tree_with_history(binary_Tree_Node *root, binary_Tree_Node *new_node) {
        if (new_node == nullptr) {
            return root;
        }
        auto insert_node = find_insert_position(root, new_node);

        if (insert_node == nullptr) {
            return new_node;
        } else if (insert_node->data < new_node->data) {
            const auto nodes = new binary_Tree_Node<T>;
            std::copy_n(insert_node, 1, nodes);
            binary_Tree_Node::replace_sub_tree_right(insert_node, new_node);
        } else {
            const auto nodes = new binary_Tree_Node<T>;
            std::copy_n(insert_node, 1, nodes);
            binary_Tree_Node::replace_sub_tree_left(nodes, new_node);
            // 从 insert_node 向上，copy 一条完整的链路，指针都是对的链路
            while (insert_node != nullptr) {
                insert_node = insert_node->parent;
            }
        }
        return root;
        // 插入完成之后，然后再重新进行排序，根据左右树的高度，看看是否需要调节高度
    }

    /**
     * 从二叉树中删除一个结点的操作，问题是不应该返回bool或者false，返回false是没有意义的
     * @param root
     * @param delete_node
     * @return 返回整个树的根结点
     */
    binary_Tree_Node *delete_node_from_binary_search_tree(binary_Tree_Node *root, binary_Tree_Node &delete_node) {
        if (&delete_node == root && delete_node.left == nullptr && delete_node.right == nullptr) {
            // 删除的是根结点，那么根结点为空
            // 只有一个结点，这里删除完成之后再去在外面free吧，因为是引用，引用最好不要修改
            // 不是引用不能修改其中的值，只是引用不能修改这个指针
            return nullptr;
        } else if (&delete_node == root && delete_node.left != nullptr && delete_node.right == nullptr) {
            return delete_node.left;
        } else if (&delete_node == root && delete_node.left == nullptr && delete_node.right != nullptr) {
            return delete_node.right;
        }
        if (delete_node.right == nullptr && delete_node.left == nullptr) {
            // 如果被删除的是叶子结点，那么就清除父结点的索引
            delete_node.clean_sub_tree_father(&delete_node);
        } else if (delete_node.right == nullptr && delete_node.left != nullptr) {
            // 右子树为空
            delete_node.replace_sub_tree(&delete_node, delete_node.left);
        } else if (delete_node.right != nullptr && delete_node.left == nullptr) {
            // 左子树为空
            delete_node.replace_sub_tree(&delete_node, delete_node.right);
        } else if (delete_node.right != nullptr && delete_node.left != nullptr) {
            // 寻找后继
            auto successor = root->tree_successor(&delete_node); // 这行还是有问题的，还是编译不过，
            if (successor->left == nullptr && successor->right == nullptr) {
                successor->clean_sub_tree_father(successor);
            } else if (successor->left == nullptr && successor->right != nullptr) {
                // 后继右子树替换掉后继原本的位置
                // 后继是删除结点的右孩子 时也是执行这个操作
                successor->replace_sub_tree(successor, successor->right);
            }
            // successor 这个时候是删除结点 一个最小的叶子结点了
            // 查找后继时就决定上一行的内容，
            // 重点时这时候 successor 是一个孤立的结点
            // 父亲，左右孩子都可以被直接赋值
            // 后继右子树替换掉被删除结点原本的位置
            successor->replace_sub_tree(&delete_node, successor);

            if (delete_node.parent == nullptr) {
                root = successor;
            }
            // 清理后继和其父亲的关系
            successor->replace_sub_tree_left(successor, delete_node.left);
            successor->replace_sub_tree_right(successor, delete_node.right);

            // 将后继与被删除的结点进行替换
            // return find_root(successor);
            //这里变更了根结点吗？并没有，所以不需要上面那一行
        }
        // 找到需要删除的后继，之后再做相应的操作，


        // 判断后继的是左右子结点都是空，然后只有左结点为空，右结点不为空
        // 如果左右结点为空，那么直接将这个后继添加到这里好，
        // 如果右结点不为空呢？之后应该怎么解决呢？
        // 将后继的右子树替代掉后继的位置，然后将后继替代掉删除的位置

        // 插入完成之后，然后再重新进行排序，根据左右树的高度，看看是否需要调节高度
        // 高度重排是红黑特有的还是二叉平衡树也有的？

        // 因为最后需要返回根结点，
        return root;
    }

    // 找到给定的值的结点的指针
    // 从给定的指针中找到前驱和后继

    // 之后才能用于一些相交的判断
    // 或者说只有先与最近的前驱或后继相交之后才会与其他的相交

    // 还有一个是寻找叶子结点的前驱和后继

    // 下面这几个方法移动到基础类中会更好

    int tree_add_after_node(struct binary_Tree_Node *root, struct binary_Tree_Node *new_node) {
        if (root == nullptr) {
            root = new_node; // 这里是添加首个数字的位置
        } else {
            struct binary_Tree_Node *new_root = root;
            struct binary_Tree_Node *miximum_leaf = new_node->find_miximum_leaf(new_root);
            while (miximum_leaf->parent->right != nullptr) {
                miximum_leaf = miximum_leaf->parent;
            }
            add_new_node_to_right(miximum_leaf->parent, new_node);
        }
    }

    int add_node_to_left_child(struct binary_Tree_Node *left) {
        this->left = left;
        left->parent = this;
    }

    int add_node_to_right_child(struct binary_Tree_Node *right) {
        this->right = right;
        right->parent = this;
    }

    static int add_new_node_before_parent(struct binary_Tree_Node *node, struct binary_Tree_Node *new_node) {
        new_node->parent = node->parent;
        new_node->left = node;
        node->parent = new_node;
        if (new_node->parent != nullptr && new_node->parent->left == node) {
            new_node->parent->left = new_node;
        } else if (new_node->parent != nullptr && new_node->parent->right == node) {
            new_node->parent->right = new_node;
        }
    }

    int add_new_node_to_right(struct binary_Tree_Node *node, struct binary_Tree_Node *new_node) {
        new_node->right = node->right; // 新节点的右孩子更新为将要原本的右孩子
        node->right = new_node; // 更新node的右孩子
        new_node->parent = node; // 更新新节点的parent
        if (new_node->right != nullptr) {
            new_node->right->parent = new_node; // 如果原本的右孩子不为空，更新原本的父节点
        }
    }
};


#endif //BALANCE_TREE_NODE_H
