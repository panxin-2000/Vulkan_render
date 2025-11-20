//
// Created by 潘鑫 on 2025/10/6.
//

#ifndef RB_TREE_NODE_H
#define RB_TREE_NODE_H
#include "binary_Tree_Node.h"
#include "RB_tree_function.h"

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


    RB_Tree_Node_color color;


    RB_Tree_Node *tree_insert_value(RB_Tree_Node *root, T data) {
        auto nodes = new RB_Tree_Node<T>;
        nodes->right = nullptr;
        nodes->left = nullptr;
        nodes->parent = nullptr;
        nodes->data = data;
        return BIN_tree::insert_node_to_RB_search_tree(root, nodes,
                                                       static_cast<RB_Tree_Node *>(nullptr),
                                                       [](RB_Tree_Node *insert_node) { return insert_node; });
    }

    std::vector<T> *translate(std::vector<RB_Tree_Node *> src_s) {
        auto result = new std::vector<T>();
        for (auto src: src_s) {
            result->push_back(src->data);
        }
        return result;
    }

    RB_Tree_Node *delete_node_from_binary_search_tree(RB_Tree_Node *root, RB_Tree_Node *delete_node) {
        return BIN_tree::delete_node_from_binary_search_tree(root, delete_node,
                                                             static_cast<RB_Tree_Node *>(nullptr),
                                                             [](RB_Tree_Node *insert_node) { return insert_node; });
    }
};

#endif //RB_TREE_NODE_H
