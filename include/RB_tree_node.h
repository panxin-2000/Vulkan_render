//
// Created by 潘鑫 on 2025/10/6.
//

#ifndef RB_TREE_NODE_H
#define RB_TREE_NODE_H
#include "binary_Tree_Node.h"


//
template<class T>
class RB_Tree_Node : public binary_Tree_Node<RB_Tree_Node<T> > {
    // 这里有一个使用模版不断套娃的小技巧
public:
    int color;

    // 持久树，持久树的目的是为了保存两个操作 persistent
    // 其实有一个问题，是否可以只用保存一个新的树到叶子结点的链而保存保存整个树中的内容呢？
    // 红黑树不行，因为不止一个从叶子到根的指针或者颜色改变，至少需要改半个树吧？或者说至少需要十个树的结点
    // 因为有时的黑高也是一个判断的选项，所以有时候需要更改半个树来重新维持持久的操作

    RB_Tree_Node *tree_insert_value(RB_Tree_Node *root, T data) {
        // 其实稍微有点不想做，为什么呢？因为感觉有点细碎
        // 但是还是需要去做的，其实重点还是去写测试的规则
        // 这里其实是有五条规则的，但是我已经并不能
        // 比如一个红色结点下的两个结点的颜色比如是黑色的
        // 树的根结点是黑色的
        // 树的黑高，

    }

    RB_Tree_Node *insert_node_to_binary_search_tree(RB_Tree_Node *root, RB_Tree_Node &new_node) {
    }

    RB_Tree_Node *delete_node_from_binary_search_tree(RB_Tree_Node *root, RB_Tree_Node &delete_node) {
    }
};


#endif //RB_TREE_NODE_H
