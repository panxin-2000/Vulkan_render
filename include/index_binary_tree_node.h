//
// Created by 潘鑫 on 2025/11/16.
//

#ifndef INDEX_BINARY_TREE_NODE_H
#define INDEX_BINARY_TREE_NODE_H
#include <vector>

#include "tree_node.h"

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
        node new_node{input_data, get_nil_v_index()};
        v_index result = get_nil_v_index();
        details.push_back(new_node);
        roots.push_back(result);
        return result;
    }

    v_index get_root_v_index() {
        if (roots.empty() == false)
            return roots.at(roots.size() - 1);
        else {
            v_index result = get_nil_v_index();
            return result;
        }
    }

    node &get_node(v_index need) {
        return details.at(need.number);
    }

    node &get_root_node() {
        return details.at(get_root_v_index().number);
    }

    v_index get_nil_v_index() {
        v_index result;
        result.number = 0;
        return result;
    }


    v_index add_new_node(T input_data) {
        if (roots.empty() == true && details.empty() == true) {
            init_root(input_data);
        }
        node new_node{input_data, get_nil_v_index()};

        v_index new_node_v_index;
        new_node_v_index.number = details.size();
        details.push_back(new_node);

        auto insert_v_index_value =
                find_insert_position(get_root_v_index(),
                                     new_node_v_index,
                                     get_nil_v_index(),
                                     std::bind(&index_binary_Tree_Node::get_node, this, std::placeholders::_1));

        if (insert_v_index_value == get_nil_v_index()) {
            roots.push_back(new_node_v_index);
            return new_node_v_index;
        } else if (get_node(insert_v_index_value).data < new_node.data) {
            get_node(insert_v_index_value).right = new_node_v_index;
        } else {
            get_node(insert_v_index_value).left = new_node_v_index;
        }
        return new_node_v_index;
    }
};

// #undef v_index

#endif //INDEX_BINARY_TREE_NODE_H
