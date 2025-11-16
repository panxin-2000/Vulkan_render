//
// Created by 潘鑫 on 2025/11/16.
//

#ifndef INDEX_BINARY_TREE_NODE_H
#define INDEX_BINARY_TREE_NODE_H
#include <vector>

#include "tree_node.h"

class index {
public:
    int number;

    friend bool operator==(index left, index right) {
        return left.number == right.number;
    }

    friend bool operator!=(index left, index right) {
        return left.number != right.number;
    }
};

template<class T, typename index>
class index_Tree_Node {
    using ptr = T *;

public:
    // index parent;
    index left;
    index right;
    T data;

    index_Tree_Node() {
    }

    index_Tree_Node(T _data, index _index) {
        data = _data;
        left = _index;
        right = _index;
    }
};

template<typename T, typename index>
class index_binary_Tree_Node {
    using node = index_Tree_Node<T, index>;
    std::vector<node> details;
    std::vector<index> roots;

public:
    index_binary_Tree_Node &get_detail_from_index(index in) {
        return details.at(in.number);
    }

    // 这里的操作是什么意思呢？
    // 只是为了初始化一个哨兵
    // 其他的操作并没有去做
    index init_root(T input_data) {
        node new_node{input_data, get_nil_index()};
        index result = get_nil_index();
        details.push_back(new_node);
        roots.push_back(result);
        return result;
    }

    index get_root_index() {
        if (roots.empty() == false)
            return roots.at(roots.size() - 1);
        else {
            index result = get_nil_index();
            return result;
        }
    }

    node &get_node(index need) {
        return details.at(need.number);
    }

    node &get_root_node() {
        return details.at(get_root_index().number);
    }

    index get_nil_index() {
        index result;
        result.number = 0;
        return result;
    }


    index find_insert_position(index root, index new_node, index nil_index) {
        index new_root = root;
        index insert_node = nil_index;
        while (new_root != nil_index) {
            insert_node = new_root;
            if (get_node(insert_node).data < get_node(new_node).data) {
                // 新插入的结点在比较的后面
                new_root = get_node(new_root).right;
            } else {
                new_root = get_node(new_root).left;
            }
        }
        return insert_node;
    }


    index add_new_node(T input_data) {
        if (roots.empty() == true && details.empty() == true) {
            init_root(input_data);
        }
        node new_node{input_data, get_nil_index()};

        index new_node_index;
        new_node_index.number = details.size();
        details.push_back(new_node);

        auto insert_index_value = find_insert_position(get_root_index(), new_node_index, get_nil_index());

        if (insert_index_value == get_nil_index()) {
            roots.push_back(new_node_index);
            return new_node_index;
        } else if (get_node(insert_index_value).data < new_node.data) {
            get_node(insert_index_value).right = new_node_index;
        } else {
            get_node(insert_index_value).left = new_node_index;
        }
        return new_node_index;
    }
};

// #undef index

#endif //INDEX_BINARY_TREE_NODE_H
