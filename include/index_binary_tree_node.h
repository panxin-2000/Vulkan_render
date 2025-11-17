//
// Created by 潘鑫 on 2025/11/16.
//

#ifndef INDEX_BINARY_TREE_NODE_H
#define INDEX_BINARY_TREE_NODE_H
#include <vector>

#include "tree_node.h"
#include "tree_function.h"


class v_index {
public:
    int number;

    friend bool operator==(v_index left, v_index right) {
        return left.number == right.number;
    }

    friend bool operator!=(v_index left, v_index right) {
        return left.number != right.number;
    }
};

template<class T>
class index_Tree_Node {
    using ptr = T *;

public:
    v_index left;
    v_index right;

public:
    T data;

    index_Tree_Node(T _data, v_index _nil_v_index) {
        data = _data;
        left = _nil_v_index;
        right = _nil_v_index;
    }
};

template<class T>
class index_Tree_Node_with_father {
    using ptr = T *;

public:
    v_index left;
    v_index right;
    v_index father;

public:
    T data;

    index_Tree_Node_with_father(T _data, v_index _nil_v_index) {
        data = _data;
        left = _nil_v_index;
        right = _nil_v_index;
        father = _nil_v_index;
    }
};


template<typename T, typename index_node>
class index_binary_Tree_Node {
    using node = index_node;
    std::vector<node> details;
    std::vector<v_index> roots;

public:
    index_binary_Tree_Node &get_detail_from_v_index(v_index in) {
        return details.at(in.number);
    }

    // 这里的操作是什么意思呢？
    // 只是为了初始化一个哨兵
    // 其他的操作并没有去做
    v_index init_root(T input_data) {
        node new_node{input_data, get_nil_index()};
        v_index result = get_nil_index();
        details.push_back(new_node);
        roots.push_back(result);
        return result;
    }

    v_index get_root_index() {
        if (roots.empty() == false)
            return roots.at(roots.size() - 1);
        else {
            v_index result = get_nil_index();
            return result;
        }
    }

    std::vector<index_node *> *translate(std::vector<v_index> src_s) {
        auto result = new std::vector<index_node *>();
        for (auto src: src_s) {
            result->push_back(get_node_ptr(src));
        }
        return result;
    }


    std::vector<v_index> *preorder_tree_walk_index() {
        auto result = new std::vector<v_index>();
        auto last_result = tree_walk_with_stack(get_root_index(),
                                                result,
                                                get_nil_index(),
                                                std::bind(&index_binary_Tree_Node::get_node, this,
                                                          std::placeholders::_1),
                                                tree_walk_type::preorder_type);
        return last_result;
    }

    node &get_node(v_index need) {
        return details.at(need.number);
    }

    node *get_node_ptr(v_index need) {
        return &details.at(need.number);
    }

    node &get_root_node() {
        return details.at(get_root_index().number);
    }

    v_index get_nil_index() {
        v_index result;
        result.number = 0;
        return result;
    }

    v_index tree_insert_value(v_index root, T data) {
        this->add_new_node(data);
        return root;
    }


    v_index add_new_node(T input_data) {
        if (roots.empty() == true && details.empty() == true) {
            init_root(input_data);
        }
        node new_node{input_data, get_nil_index()};

        v_index new_node_v_index;
        new_node_v_index.number = details.size();
        details.push_back(new_node);

        auto insert_v_index_value =
                find_insert_position(get_root_index(),
                                     new_node_v_index,
                                     get_nil_index(),
                                     std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));

        if (insert_v_index_value == get_nil_index()) {
            roots.push_back(new_node_v_index);
            return new_node_v_index;
        } else if (get_node(insert_v_index_value).data < new_node.data) {
            get_node(insert_v_index_value).right = new_node_v_index;
        } else {
            get_node(insert_v_index_value).left = new_node_v_index;
        }
        return new_node_v_index;
    }


    v_index delete_node_from_binary_search_tree(v_index delete_node) {
        auto root = get_root_index();
        if (delete_node == get_root_index() && get_node(delete_node).left == get_nil_index() && get_node(delete_node).
            right == get_nil_index()) {
            return get_nil_index();
        } else if (delete_node == root && get_node(delete_node).left != get_nil_index() && get_node(delete_node).right
                   ==
                   get_nil_index()) {
            return get_node(delete_node).left;
        } else if (delete_node == root && get_node(delete_node).left == get_nil_index() && get_node(delete_node).right
                   !=
                   get_nil_index()) {
            return get_node(delete_node).right;
        }
        if (get_node(delete_node).right == get_nil_index() && get_node(delete_node).left == get_nil_index()) {
            // 如果被删除的是叶子结点，那么就清除父结点的索引
            clean_sub_tree_father(delete_node, get_nil_index(),
                                  std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));
        } else if (get_node(delete_node).right == get_nil_index() && get_node(delete_node).left != get_nil_index()) {
            // 右子树为空
            replace_sub_tree(delete_node, get_node(delete_node).left,
                             std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));
        } else if (get_node(delete_node).right != get_nil_index() && get_node(delete_node).left == get_nil_index()) {
            // 左子树为空
            replace_sub_tree(delete_node, get_node(delete_node).right,
                             std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));
        } else if (get_node(delete_node).right != get_nil_index() && get_node(delete_node).left != get_nil_index()) {
            // 寻找后继
            auto successor = tree_successor(delete_node,
                                            std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));
            // 这行还是有问题的，还是编译不过，
            if (successor->left == get_nil_index() && successor->right == get_nil_index()) {
                clean_sub_tree_father(successor,
                                      std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));
            } else if (successor->left == get_nil_index() && successor->right != get_nil_index()) {
                replace_sub_tree(successor, successor->right,
                                 std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));
            }
            replace_sub_tree(delete_node, successor,
                             std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));

            if (get_node(delete_node).parent == get_nil_index()) {
                root = successor;
            }
            replace_sub_tree_left(successor, get_node(delete_node).left,
                                  std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));
            replace_sub_tree_right(successor, get_node(delete_node).right,
                                   std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));
        }
        return root;
    }
};


// #undef v_index

#endif //INDEX_BINARY_TREE_NODE_H
