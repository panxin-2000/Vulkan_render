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

    void inorder_tree_walk(T *node) {
        if (node != nullptr) {
            inorder_tree_walk(node->left);
            std::cout << node->data << std::endl;
            inorder_tree_walk(node->right);
        }
    }

    void preorder_tree_walk(T *node) {
        if (node != nullptr) {
            std::cout << node->data << std::endl;
            preorder_tree_walk(node->left);
            preorder_tree_walk(node->right);
        }
    }

    void postorder_tree_walk(T *node) {
        if (node != nullptr) {
            postorder_tree_walk(node->left);
            postorder_tree_walk(node->right);
            std::cout << node->data << std::endl;
        }
    }

    std::vector<binary_Tree_Node<T> *> find_interval(T &left_node, T &right_node) {
        //
        auto new_left_node = left_node;
        auto new_right_node = right_node;
        if (right_node < left_node) {
            std::swap(new_left_node, new_right_node);
        }
        auto root = this;
        std::vector<binary_Tree_Node<T> *> result = *new std::vector<binary_Tree_Node<T> *>;
        // 找到最小值和最大值
        auto min_node = root->tree_find_value(root, new_left_node); //其实这里是稍微有点问题的
        auto max_node = root->tree_find_value(root, new_right_node); //大部分情况是是取一个间隔，并不能准确的对应的值
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

    void level_tree_walk(T *node) {
        std::queue<T *> tem;
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
            std::cout << node_tem->data << std::endl;
            tem.pop();
        }
    }

public:
    // 持久树，持久树的目的是为了保存两个操作 persistent
    // 其实有一个问题，是否可以只用保存一个新的树到叶子结点的链而保存保存整个树中的内容呢？
    // 这里是没有问题的，从那个结点向上到根之间是需要保存的
    //
    binary_Tree_Node *tree_insert_value(binary_Tree_Node *root, T data) {
        binary_Tree_Node &nodes = *new binary_Tree_Node<T>;
        nodes.right = nullptr;
        nodes.left = nullptr;
        nodes.parent = nullptr;
        nodes.data = data;
        return insert_node_to_binary_search_tree(root, nodes);
    }

    binary_Tree_Node *tree_find_value(binary_Tree_Node *root, T data) {
        binary_Tree_Node *new_root = root;
        binary_Tree_Node *result_node = nullptr;
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

    /**
     * 确定一下返回值，返回值总是返回树的根
     * @param root
     * @param new_node
     * @return
     */
    binary_Tree_Node *insert_node_to_binary_search_tree(binary_Tree_Node *root, binary_Tree_Node &new_node) {
        binary_Tree_Node *new_root = root;
        binary_Tree_Node *insert_node = nullptr;
        while (new_root != nullptr) {
            insert_node = new_root;
            if (insert_node->data < new_node.data) {
                new_root = new_root->right;
            } else {
                new_root = new_root->left;
            }
        }
        if (insert_node == nullptr) {
            return &new_node;
        } else if (insert_node->data < new_node.data) {
            insert_node->right = &new_node;
            new_node.parent = insert_node;
        } else {
            insert_node->left = &new_node;
            new_node.parent = insert_node;
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
        }
        if (delete_node.right == nullptr && delete_node.left == nullptr) {
            // 如果被删除的是叶子结点，那么就清除父结点的索引
            if (delete_node.parent->left == &delete_node) {
                delete_node.parent->left = nullptr;
            } else if (delete_node.parent->right == &delete_node) {
                delete_node.parent->right = nullptr;
            }
        } else if (delete_node.right == nullptr && delete_node.left != nullptr) {
            // 右子树为空
            if (delete_node.parent->left == &delete_node) {
                // 为父亲的左子树
                delete_node.parent->left = delete_node.left; // 将父亲的左子树更新为删除节点的左子树
                delete_node.left->parent = delete_node.parent; // 将删除结点的左子树的父亲更新为删除结点的父亲
            } else if (delete_node.parent->right == &delete_node) {
                // 就不继续注释了
                delete_node.parent->right = delete_node.left;
                delete_node.left->parent = delete_node.parent;
            }
        } else if (delete_node.right != nullptr && delete_node.left == nullptr) {
            // 左子树为空
            if (delete_node.parent->left == &delete_node) {
                delete_node.parent->left = delete_node.right;
                delete_node.right->parent = delete_node.parent;
            } else if (delete_node.parent->right == &delete_node) {
                delete_node.parent->right = delete_node.right;
                delete_node.right->parent = delete_node.parent;
            }
        } else if (delete_node.right != nullptr && delete_node.left != nullptr) {
            // 寻找后继
            auto successor = root->tree_successor(&delete_node); // 这行还是有问题的，还是编译不过，
            if (successor->left == nullptr && successor->right == nullptr) {
                if (successor->parent->left == successor) {
                    successor->parent->left = nullptr;
                } else if (successor->parent->right == successor) {
                    successor->parent->right = nullptr;
                }
            } else if (successor->left == nullptr && successor->right != nullptr) {
                if (successor->parent == &delete_node) {
                    // 后继是删除结点的右孩子， // 想办法跳过就可以了
                    delete_node.right = successor->right;
                } else {
                    // 后继不是删除结点的右孩子
                    successor->parent->left = successor->right;
                    successor->right->parent = successor->parent;
                }
            }
            // successor 这个时候是算法一个最小的叶子结点了
            successor->parent = delete_node.parent;

            if (delete_node.parent != nullptr && delete_node.parent->left == &delete_node) {
                delete_node.parent->left = successor;
            } else if (delete_node.parent != nullptr && delete_node.parent->right == &delete_node) {
                delete_node.parent->right = successor;
            }
            if (delete_node.parent == nullptr) {
                root = successor;
            }
            // 清理后继和其父亲的关系
            successor->left = delete_node.left;
            successor->left->parent = successor;

            successor->right = delete_node.right;
            successor->right->parent = successor;
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
