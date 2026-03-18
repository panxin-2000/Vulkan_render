//
// Created by 潘鑫 on 2025/12/21.
//

#ifndef HELLO_MAC_QUAD_BASE_TREE_H
#define HELLO_MAC_QUAD_BASE_TREE_H
#include "iostream"
#include "base_element/base.h"
#include "quadtree_node.h"


template<typename AABB_box, typename index_node, size_t grid_size = 10>
class quad_tree_base {
public:
    using node    = Quad_Tree_Node<Point_2>;
    using v_index = u_int16_t;
    std::vector<v_index> roots;
    std::vector<node> details;
    v_index delete_v_index = 0;

    node *get_node_from_v_index(v_index in) {
        if (in == get_nil_index()) {
            return nullptr;
        }
        return &details.at(in);
    }

    quad_tree_base(Point_2 centroid_point, Point_2 direction_interval) {
        details.push_back({centroid_point, direction_interval});
        v_index result = get_nil_index();
        roots.push_back(result);
    }

    Point_2 get_node_centroid_point(v_index node) {
        return details.at(node).centroid_point;
    }

    Point_2 get_node_direction_interval(v_index node) {
        return details.at(node).direction_interval;
    }

private:
    void set_node_parent(const v_index node, const v_index parent) {
        details.at(node).node[0] = parent;
    }

    void set_node_left_down(const v_index node, const v_index left_down) {
        details.at(node).node[1] = left_down;
    }

    void set_node_left_up(const v_index node, const v_index left_up) {
        details.at(node).node[3] = left_up;
    }

    void set_node_right_down(const v_index node, const v_index right_down) {
        details.at(node).node[2] = right_down;
    }

    void set_node_right_up(const v_index node, const v_index right_up) {
        details.at(node).node[4] = right_up;
    }

    //  3 4
    //  1 2

    v_index add_left_down_node(v_index node) {
        v_index new_node   = details.size();
        auto node_centroid = get_node_centroid_point(node);
        auto direction     = get_node_direction_interval(node);
        node_centroid      = node_centroid - direction / 2;
        details.push_back({node_centroid, get_node_direction_interval(node) / 2});
        set_node_left_down(node, new_node);
        set_node_parent(new_node, node);
    }

    v_index add_right_up_node(v_index node) {
        v_index new_node   = details.size();
        auto node_centroid = get_node_centroid_point(node);
        auto direction     = get_node_direction_interval(node);
        node_centroid      = node_centroid + direction / 2;
        details.push_back({node_centroid, get_node_direction_interval(node) / 2});
        set_node_right_up(node, new_node);
        set_node_parent(new_node, node);
    }

    v_index add_left_up_node(v_index node) {
        v_index new_node   = details.size();
        auto node_centroid = get_node_centroid_point(node);
        auto direction     = get_node_direction_interval(node);
        node_centroid      = {node_centroid.x - direction.x / 2, node_centroid.y + direction.y / 2};
        details.push_back({node_centroid, get_node_direction_interval(node) / 2});
        set_node_left_up(node, new_node);
        set_node_parent(new_node, node);
    }

    v_index add_right_down_node(v_index node) {
        v_index new_node   = details.size();
        auto node_centroid = get_node_centroid_point(node);
        auto direction     = get_node_direction_interval(node);
        node_centroid      = {node_centroid.x + direction.x / 2, node_centroid.y - direction.y / 2};
        details.push_back({node_centroid, get_node_direction_interval(node) / 2});
        set_node_right_down(node, new_node);
        set_node_parent(new_node, node);
    }

    v_index add_left_down_parent_node(v_index node) {
        v_index new_node   = details.size();
        auto node_centroid = get_node_centroid_point(node);
        auto direction     = get_node_direction_interval(node);
        node_centroid      = node_centroid - direction;
        details.push_back({node_centroid, get_node_direction_interval(node) * 2});
        set_node_parent(node, new_node);
        set_node_right_up(new_node, node);
        if (node == get_root_index()) {
            roots.push_back(new_node);
        }
    }

    v_index add_right_up_parent_node(v_index node) {
        v_index new_node   = details.size();
        auto node_centroid = get_node_centroid_point(node);
        auto direction     = get_node_direction_interval(node);
        node_centroid      = node_centroid + direction;
        details.push_back({node_centroid, get_node_direction_interval(node) * 2});
        set_node_parent(node, new_node);
        set_node_left_down(new_node, node);
        if (node == get_root_index()) {
            roots.push_back(new_node);
        }
    }

    v_index add_left_up_parent_node(v_index node) {
        v_index new_node   = details.size();
        auto node_centroid = get_node_centroid_point(node);
        auto direction     = get_node_direction_interval(node);
        node_centroid      = {node_centroid.x - direction.x, node_centroid.y + direction.y};
        details.push_back({node_centroid, get_node_direction_interval(node) * 2});
        set_node_parent(node, new_node);
        set_node_right_down(new_node, node);
        if (node == get_root_index()) {
            roots.push_back(new_node);
        }
    }

    //   node
    //         new_node
    v_index add_right_down_parent_node(v_index node) {
        v_index new_node   = details.size();
        auto node_centroid = get_node_centroid_point(node);
        auto direction     = get_node_direction_interval(node);
        node_centroid      = {node_centroid.x + direction.x, node_centroid.y - direction.y};
        details.push_back({node_centroid, get_node_direction_interval(node) * 2});
        set_node_parent(node, new_node);
        set_node_left_up(new_node, node);
        if (node == get_root_index()) {
            roots.push_back(new_node);
        }
    }


    // 添加的时候有两种添加方式，一种是如果超出了这个格子，就添加到上一层的格子中
    //    *  *  *
    //    *     *
    //    *  *  *
    // 另一种是，添加还是还是按照大小来进行的，但是查找的时候你需要搜寻旁边的内容
    //
    v_index add_AABB_to_quad_tree(AABB_box box) {
        auto node_centroid = get_centroid_point(box);
        auto direction     = get_direction_interval(box);

        v_index root_index = get_root_index();
        while (root_index != get_nil_index()) {
            // if (get_node_centroid_point(root_index) )
        }
    }

    [[nodiscard]] v_index get_root_index() const {
        if (roots.empty() == false)
            return roots.at(roots.size() - 1);
        else {
            v_index result = get_nil_index();
            return result;
        }
    }

    std::vector<index_node *> *translate(const std::vector<v_index> &src_s) {
        auto result = new std::vector<index_node *>();
        for (auto src: src_s) {
            result->push_back(get_node_ptr(src));
        }
        return result;
    }

    std::vector<AABB_box> *translate_data(const std::vector<v_index> &src_s) {
        auto result = new std::vector<AABB_box>();
        for (auto src: src_s) {
            result->push_back(get_node_ptr(src)->data);
        }
        return result;
    }

    v_index tree_find_value(AABB_box data) {
        return BIN_tree::tree_find_value(get_root_index(), data,
                                         get_nil_index(),
                                         std::bind(&index_tree_base::get_node_ptr, this, std::placeholders::_1));
    }

    std::vector<v_index> *inorder_tree_walk_with_stack(std::vector<v_index> *result) {
        return tree_walk_with_stack(get_root_index(), result,
                                    get_nil_index(),
                                    std::bind(&index_tree_base::get_node_ptr, this,
                                              std::placeholders::_1), tree_walk_type::inorder_type);
    }


    std::vector<v_index> *preorder_tree_walk_index() {
        auto result      = new std::vector<v_index>();
        auto last_result = tree_walk_with_stack(get_root_index(),
                                                result,
                                                get_nil_index(),
                                                std::bind(&index_tree_base::get_node_ptr, this,
                                                          std::placeholders::_1),
                                                tree_walk_type::preorder_type);
        return last_result;
    }


    node *get_node_ptr(v_index need) {
        if (need == get_nil_index()) {
            return nullptr;
        }
        return &details.at(need);
    }

    node *get_root_node() {
        return &details.at(get_root_index());
    }

    [[nodiscard]] static v_index get_nil_index() {
        return 0;
    }

    // 这里的本质是一个链表，虽然已经使用过了，但是并不删除，只是标记并没有被使用，新插入时占据原本的位置
    bool update_new_delete_node(v_index delete_node_index) {
        get_node_ptr(delete_node_index)->left = delete_v_index;
        delete_v_index                        = delete_node_index;
        return true;
    }

    // 这里的本质是一个链表，虽然已经使用过了，但是并不删除，只是标记并没有被使用，新插入时占据原本的位置
    v_index get_one_delete_node() {
        auto new_node_v_index = delete_v_index;
        delete_v_index        = get_node_ptr(new_node_v_index)->left;
        return new_node_v_index;
    }

    v_index get_new_node_index(AABB_box input_data) {
        if (roots.empty() == true && details.empty() == true) {
            init_root(input_data);
            delete_v_index = get_nil_index();
        }

        v_index new_node_v_index;
        if (delete_v_index == get_nil_index()) {
            node new_node{input_data, get_nil_index()};
            new_node_v_index = details.size();
            details.push_back(new_node);
        } else {
            new_node_v_index = get_one_delete_node();
            index_node::init_index_node(get_node_ptr(new_node_v_index),
                                        input_data,
                                        get_nil_index());
        }
        return new_node_v_index;
    }

    AABB_box *get_data(v_index index) {
        auto temp = get_node_from_v_index(index);
        if (temp != nullptr)
            return &temp->data;
        return nullptr;
    }
};
#endif //HELLO_MAC_QUAD_BASE_TREE_H
