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
        while (std::numeric_limits<uint32_t>::max() != data[node_index_current].next_entities_index) {
            data[node_index_current].get_all_entity(result);
            node_index_current = data[node_index_current].next_entities_index;
        }
        return true;
    }

    template<typename Point_type>
    [[nodiscard]] bool Quadtree<Point_type>::add_entity(const uint32_t node_index, const entt::entity entity) {
        assert(node_index< data.size());
        bool flag                   = false;
        uint32_t node_index_current = node_index;
        if (f_set_entity_node_index_ != nullptr) {
            f_set_entity_node_index_(entity, node_index);
        }
        while (data[node_index_current].add_entity(entity) == false) {
            if (std::numeric_limits<uint32_t>::max() == data[node_index_current].next_entities_index) {
                data[node_index_current].next_entities_index = data.size();
                data.emplace_back();
            }
            node_index_current = data[node_index_current].next_entities_index;
        }
        return true;
    }

    template<typename Point_type>
    [[nodiscard]] bool Quadtree<Point_type>::remove_entity(const uint32_t node_index, const entt::entity entity) {
        // Find the value in node->values
        assert(node_index< data.size());
        uint32_t node_index_current = node_index;
        while (data[node_index_current].remove_entity(entity) == false) {
            if (std::numeric_limits<uint32_t>::max() != data[node_index_current].next_entities_index) {
                node_index_current = data[node_index_current].next_entities_index;
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
        uint32_t current_node_index = node_index;
        auto current_node_box       = node_box;
        while (ideal_depth > current_depth) {
            const auto i       = get_quadrant(current_node_box, entity_box);
            current_node_box   = compute_Box_position_size(current_node_box, i);
            current_node_index = data[current_node_index].children_index[i];
            current_depth      = current_depth + 1;
        }
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
            } else if (ideal_depth > current_depth && isLeaf(current_node_index)) {
                split(current_node_index);
                const auto i       = get_quadrant(current_node_box, entity_box);
                current_node_box   = compute_Box_position_size(current_node_box, i);
                current_node_index = data[current_node_index].children_index[i];
                current_depth      = current_depth + 1;
            } else {
                const auto i       = get_quadrant(current_node_box, entity_box);
                current_node_box   = compute_Box_position_size(current_node_box, i);
                current_node_index = data[current_node_index].children_index[i];
                current_depth      = current_depth + 1;
            }
        }
        return false;
    }

    template<typename Point_type>
    void Quadtree<Point_type>::split(const uint32_t node_index) {
        assert(isLeaf(node_index) && "Only leaves can be split");
        for (auto &child: data[node_index].children_index) {
            assert(data.size() < max_quadtree_node);
            child = data.size();
            data.emplace_back();
            node_size_++;
        }
    }


    template<typename Point_type>
    void Quadtree<Point_type>::query_one_node(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                                              const AABB_centroid<Point_type> &check_box,
                                              std::vector<entt::entity> &result) const {
        for (const auto &value: data[node_index].entities) {
            if (value != entt::null) {
                const AABB_centroid<Point_type> entity_box{f_get_AABB_centroid_(value), f_get_AABB_radius_(value)};
                if (is_intersect(entity_box, check_box))
                    result.push_back(value);
            }
        }
    }

    template<typename Point_type>
    void Quadtree<Point_type>::query(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                                     const AABB_centroid<Point_type> &check_box,
                                     std::vector<entt::entity> &result) const {
        // 简单 当前 node_index
        query_one_node(node_index, node_box, check_box, result);
        // 检查 全部的 children node_index
        for (auto i = 0; i < data[node_index].children_index.size(); ++i) {
            auto child = data[node_index].children_index[i];
            if (child != std::numeric_limits<uint32_t>::max()) {
                const auto child_box          = compute_Box_position_size(node_box, static_cast<sub_AABB>(i));
                const auto child_loose_bounds = AABB_centroid<Point_type>{
                    child_box.get_centroid_point(), child_box.get_radius() * 2
                };
                if (is_internal(check_box, child_loose_bounds)) {
                    // 如果在内部的话，那么直接全部添加
                    get_all_entity(node_index, result);
                } else {
                    // 如果是边界的话，
                    query(child, node_box, check_box, result);
                }
            }
        }
    }
}


#endif //HELLO_MAC_ENTITY_QUADTREE_IMPL_H
