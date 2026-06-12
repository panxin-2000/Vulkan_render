//
// Created by 潘鑫 on 2026/6/12.
//

#ifndef HELLO_MAC_ENTITY_QUADTREE_IMPL_H
#define HELLO_MAC_ENTITY_QUADTREE_IMPL_H
#include "entity_quadtree.h"


namespace ECS {
    template<typename Point_type>
    bool Quadtree<Point_type>::get_all_entity(const uint32_t node_index, std::vector<entt::entity> &result) const {
        uint32_t node_index_current = node_index;
        while (std::numeric_limits<uint32_t>::max() != get_next_node_index(node_index_current)) {
            get_node(node_index_current).get_all_entity(result);
            node_index_current = get_next_node_index(node_index_current);
        }
        return true;
    }

    template<typename Point_type>
    [[nodiscard]] bool Quadtree<Point_type>::clean(const uint32_t node_index) {
        uint32_t node_index_current = node_index;
        while (get_node(node_index_current).empty() == true) {
            if (std::numeric_limits<uint32_t>::max() != get_next_node_index(node_index_current)) {
                auto need_clean    = node_index_current;
                node_index_current = get_next_node_index(node_index_current);
                get_node(need_clean).clean();
                free_list.emplace_back(need_clean);
                node_size_--;
            } else {
                return true;
            }
        }
        return false;
    }

    template<typename Point_type>
    [[nodiscard]] bool Quadtree<Point_type>::is_empty(const uint32_t node_index) const {
        uint32_t node_index_current = node_index;
        while (get_node(node_index_current).empty() == true) {
            if (std::numeric_limits<uint32_t>::max() != get_next_node_index(node_index_current)) {
                node_index_current = get_next_node_index(node_index_current);
            } else {
                return true;
            }
        }
        return false;
    }

    template<typename Point_type>
    [[nodiscard]] bool Quadtree<Point_type>::add_entity(const uint32_t node_index, const entt::entity entity) {
        assert(node_index< data.size());
        bool flag                   = false;
        uint32_t node_index_current = node_index;
        while (get_node(node_index_current).add_entity(entity) == false) {
            if (std::numeric_limits<uint32_t>::max() == get_next_node_index(node_index_current)) {
                if (!free_list.empty()) {
                    set_next_node_index(node_index_current, free_list.back());
                    free_list.pop_back();
                } else {
                    set_next_node_index(node_index_current, data.size());
                    data.emplace_back();
                }
            }
            node_index_current = get_next_node_index(node_index_current);
        }
        return true;
    }

    template<typename Point_type>
    [[nodiscard]] bool Quadtree<Point_type>::remove_entity(const uint32_t node_index, const entt::entity entity) {
        // Find the value in node->values
        assert(node_index< data.size());
        uint32_t node_index_current = node_index;
        while (get_node(node_index_current).remove_entity(entity) == false) {
            if (std::numeric_limits<uint32_t>::max() != get_next_node_index(node_index_current)) {
                node_index_current = get_next_node_index(node_index_current);
            } else {
                return false;
            }
        }
        return true;
    }

    template<typename Point_type>
    bool Quadtree<Point_type>::remove_node(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                                           const entt::entity entity, const AABB_centroid<Point_type> &entity_box,
                                           uint32_t current_depth, const uint32_t ideal_depth) {
        assert(current_node_index != std::numeric_limits<uint32_t>::max());
        uint32_t current_node_index = node_index;
        auto current_node_box       = node_box;
        std::stack<uint32_t> node_stack;
        node_stack.push(current_node_index);
        while (ideal_depth > current_depth) {
            const auto i       = get_quadrant(current_node_box, entity_box);
            current_node_box   = compute_Box_position_size(current_node_box, i);
            current_node_index = get_node_child_index(current_node_index, i);
            assert(current_node_index != std::numeric_limits<uint32_t>::max());
            current_depth = current_depth + 1;
            node_stack.push(current_node_index);
        }
        // while (!node_stack.empty()) {

        // }


        if (current_node_index != std::numeric_limits<uint32_t>::max())
            return remove_entity(current_node_index, entity);
        return false;
    }

    template<typename Point_type>
    bool Quadtree<Point_type>::add_node(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                                        const entt::entity entity, const AABB_centroid<Point_type> &entity_box,
                                        uint32_t current_depth, const uint32_t ideal_depth) {
        // current_depth 为零
        uint32_t current_node_index = node_index;
        auto current_node_box       = node_box;
        while (true) {
            if (ideal_depth == current_depth) {
                return add_entity(current_node_index, entity);
            } else if (ideal_depth > current_depth && is_leaf(current_node_index)) {
                split(current_node_index);
                const auto i       = get_quadrant(current_node_box, entity_box);
                current_node_box   = compute_Box_position_size(current_node_box, i);
                current_node_index = get_node_child_index(current_node_index, i);
                current_depth      = current_depth + 1;
            } else {
                const auto i       = get_quadrant(current_node_box, entity_box);
                current_node_box   = compute_Box_position_size(current_node_box, i);
                current_node_index = get_node_child_index(current_node_index, i);
                current_depth      = current_depth + 1;
            }
        }
        return false;
    }

    template<typename Point_type>
    void Quadtree<Point_type>::split(const uint32_t node_index) {
        assert(isLeaf(node_index) && "Only leaves can be split");
        for (auto &child: get_node(node_index).children_index) {
            assert(data.size() < max_quadtree_node);
            if (!free_list.empty()) {
                child = free_list.back();
                free_list.pop_back();
            } else {
                child = data.size();
                data.emplace_back();
            }
            node_size_++;
        }
    }
}


#endif //HELLO_MAC_ENTITY_QUADTREE_IMPL_H
