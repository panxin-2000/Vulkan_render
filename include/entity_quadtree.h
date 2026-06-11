//
// Created by 潘鑫 on 2026/6/11.
//

#ifndef HELLO_MAC_ENTITY_QUADTREE_H
#define HELLO_MAC_ENTITY_QUADTREE_H


/**
* 计算物体在松散四叉树中的目标层级
* @param root_size    根节点的逻辑边长 (L0)
* @param max_depth    四叉树允许的最大深度 (防止物体太小导致层级溢出)
* @param max_radius   拿到物体的最大边长
* @return             目标层级 (0 代表根节点，数值越大越深)
*/
inline uint32_t calculate_level(const float root_size,
                                const float max_radius,
                                const uint32_t max_depth) {
    // 1. 拿到物体的最大边长
    const float max_dim = max_radius; // std::max(aabb_w, aabb_h);

    // 安全边界控制：如果物体比根节点还大，直接扔在根节点 (Level 0)
    if (max_dim >= root_size / 2.0f) {
        return 0;
    }

    // 2. 数学公式：m = log2(root_size / max_dim) - 1
    // 现代高性能写法：利用位运算或 std::log2f
    // 这里使用标准的高效浮点数 log2 向上取整/向下取整
    float target_level_f = std::log2f(root_size / max_dim) - 1.0f;
    int target_level     = static_cast<int>(std::floor(target_level_f));

    // 3. 边界限定：不能小于 0 层，也不能超过系统设定的最大深度
    if (target_level < 0) return 0;
    return std::min(static_cast<std::uint32_t>(target_level), max_depth);
}


#include <array>
#include <cstddef>

#include "Box.h"
#include "entt/entt.hpp"
#include "base_geometry/base.h"
#include "base_geometry/intersect_function.h"

namespace ECS {
    template<typename T, std::size_t N>
    constexpr std::array<T, N> make_filled_array(const T &value) {
        std::array<T, N> arr{};
        for (std::size_t i = 0; i < N; ++i) {
            arr[i] = value;
        }
        return arr;
    }

    class Quadtree_node {
    public:
        std::array<uint32_t, 4> children_index = make_filled_array<uint32_t, 4>(std::numeric_limits<uint32_t>::max());
        std::array<entt::entity, 8> entities   = make_filled_array<entt::entity, 8>(entt::null);
        // std::vector<entt::entity> vector_entities;
        uint32_t next_entities_index = std::numeric_limits<uint32_t>::max();


        bool add_entity(const entt::entity entity) {
            for (auto &entity_ref: entities) {
                if (entity_ref == entt::null) {
                    entity_ref = entity;
                    return true;
                }
            }
            return false;
        }

        bool remove_entity(const entt::entity entity) {
            for (size_t i = 0; i < entities.size(); ++i) {
                if (entities[i] == entity) {
                    entities[i]     = entities.back();
                    entities.back() = entt::null;
                    return true;
                }
            }
            return false;
        }
    };

    Point_2 get_AABB_centroid(entt::entity entity) {
    }

    template<typename Point_type>
    class Quadtree {
    public:
        using Get_centroid = std::function<Point_type(entt::entity entity)>;
        using get_radius   = std::function<Point_type(entt::entity entity)>;

        explicit Quadtree(const AABB_centroid<Point_type> &box,
                          const Get_centroid &get_AABB_centroid,
                          const get_radius &get_AABB_radius) : mRootBox_position_size(box),
                                                               get_AABB_centroid_(get_AABB_centroid),
                                                               get_AABB_radius_(get_AABB_radius) {
            data.reserve(max_quadtree_node);
            data.emplace_back();
        }

        const uint32_t max_quadtree_node = 100; // 最多允许的四叉树 结点数量
        std::vector<Quadtree_node> data;
        AABB_centroid<Point_type> mRootBox_position_size;
        uint32_t mRoot = 0;

        bool add_entity(const entt::entity entity, const AABB_centroid<Point_type> &entity_box) {
            if constexpr (std::is_same_v<decltype(entity_box), const AABB_centroid<Point_2> &>) {
                auto ideal_level = calculate_level(mRootBox_position_size.get_radius().get_max_x_or_y(),
                                                   entity_box.get_radius().get_max_x_or_y(),
                                                   MaxDepth);
                return add_node(0, mRootBox_position_size, entity, entity_box, 0, ideal_level);
            } else if constexpr (std::is_same_v<decltype(entity_box), const AABB_centroid<Point_3> &>) {
                auto ideal_level = calculate_level(mRootBox_position_size.get_radius().get_max_x_or_y(),
                                                   entity_box.get_radius().get_max_x_or_y(),
                                                   MaxDepth);
                return add_node(0, mRootBox_position_size, entity, entity_box, 0, ideal_level);
            }
            assert(false && "not implemented");
            return false;
        }

        std::vector<entt::entity> query(const AABB_centroid<Point_type> &check_box) {
            std::vector<entt::entity> result;
            query(0, mRootBox_position_size, check_box, result);
            return result;
        }


        bool remove_entity(const entt::entity entity, const AABB_centroid<Point_type> &entity_box) {
            return remove_node(0, mRootBox_position_size, entity, entity_box);
        }

        [[nodiscard]] bool isLeaf(const uint32_t node_index) const {
            const auto node = data[node_index];
            if (node.children_index == make_filled_array<uint32_t, 4>(std::numeric_limits<uint32_t>::max()))
                return true;
            return false;
        }

        /**
         * 序号小的 entity 在前，序号大的在后，方便查找，允许重复
         * @return
         */
        std::multimap<entt::entity, entt::entity> find_all_intersections() {
            return std::multimap<entt::entity, entt::entity>();
        }

        /**
         * 需要确定坐标轴
         * x轴向屏幕右边方向
         * y轴向屏幕上方方向
         * z轴从屏幕指向眼睛
         * 带z的表示负的z方向
         */
        enum sub_AABB {
            invalid = -1,
            North_West,
            North_East,
            South_West,
            South_East,
            North_West_z,
            North_East_z,
            South_West_z,
            South_East_z,
        };

        /**
         *
         * @param box 大的包围盒
         * @param i 上下左右 四个 子包围盒的 索引
         * @return 子包围的大小与范围
         */
        static AABB_centroid<Point_2> compute_Box_position_size(const AABB_centroid<Point_2> &box, const sub_AABB i) {
            auto point_xy = Point_2{-1.0f * (static_cast<float>(i / 2) - 0.5f), static_cast<float>(i % 2) - 0.5f};
            return {box.centroid_point_ + box.direction_interval_ * point_xy, box.direction_interval_ * 0.5f};
        }

        static AABB_centroid<Point_3> compute_Box_position_size(const AABB_centroid<Point_3> &box, const sub_AABB i) {
            auto point_xy = Point_3{
                -1.0f * (static_cast<float>(i / 2) - 0.5f),
                +1.0f * (static_cast<float>(i % 2) - 0.5f),
                -1.0f * (static_cast<float>(i / 4) - 0.5f)
            };
            return {box.centroid_point_ + box.direction_interval_ * point_xy, box.direction_interval_ * 0.5f};
        }

        [[nodiscard]] sub_AABB get_quadrant(const AABB_centroid<Point_3> &node_box,
                                            const AABB_centroid<Point_3> &entity_box) const {
            auto bool_x = static_cast<uint32_t>(entity_box.centroid_point_.x >= node_box.centroid_point_.x);
            auto bool_y = static_cast<uint32_t>(entity_box.centroid_point_.y < node_box.centroid_point_.y);
            auto bool_z = static_cast<uint32_t>(entity_box.centroid_point_.z < node_box.centroid_point_.z);
            return static_cast<sub_AABB>(bool_x + bool_y * 2 + bool_z * 4);
        }

        /**
        *
        * @param entity_box 被检索的包围盒
        * @param node_box 需要查找的包围
        * @return 在被检索的包围盒的 上下左右的哪个位置
        */
        [[nodiscard]] sub_AABB get_quadrant(const AABB_centroid<Point_2> &node_box,
                                            const AABB_centroid<Point_2> &entity_box) const {
            const auto bool_x = static_cast<uint32_t>(entity_box.centroid_point_.x >= node_box.centroid_point_.x);
            const auto bool_y = static_cast<uint32_t>(entity_box.centroid_point_.y < node_box.centroid_point_.y);
            return static_cast<sub_AABB>(bool_x + bool_y * 2);
        }

    private:
        Get_centroid get_AABB_centroid_;
        get_radius get_AABB_radius_;
        static constexpr auto MaxDepth = static_cast<std::size_t>(8);


        /**
         * 确定性的将 entity 添加到 node_index 中
         * @param node_index
         * @param entity
         * @return
         */
        [[nodiscard]] bool add_entity(const uint32_t node_index, const entt::entity entity) {
            assert(node_index< data.size());
            bool flag                   = false;
            uint32_t node_index_current = node_index;
            while (data[node_index_current].add_entity(entity) == false) {
                if (std::numeric_limits<uint32_t>::max() == data[node_index_current].next_entities_index) {
                    data[node_index_current].next_entities_index = data.size();
                    data.emplace_back();
                }
                node_index_current = data[node_index_current].next_entities_index;
            }
            return true;
        }

        /**
         * 确定性删除 entity 在 node_index 返回 true ，否则返回 false
         * @param node_index
         * @param entity
         * @return
         */
        [[nodiscard]] bool remove_entity(const uint32_t node_index, const entt::entity entity) {
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


        bool remove_node(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                         const entt::entity entity, const AABB_centroid<Point_type> &entity_box) {
            if (!isLeaf(node_index)) {
                // Remove the value from node
                if (remove_entity(node_index, entity))
                    return true;
                const auto i = get_quadrant(node_box, entity_box);
                return remove_node(data[node_index].children_index[i],
                                   compute_Box_position_size(node_box, i), entity, entity_box);
            }
        }

        bool add_node(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                      const entt::entity entity, const AABB_centroid<Point_type> &entity_box,
                      const uint32_t current_depth, const uint32_t ideal_depth = 0) {
            if (ideal_depth == current_depth) {
                return add_entity(node_index, entity);
            } else if (isLeaf(node_index)) {
                split(node_index, node_box);
                return add_node(node_index, node_box, entity, entity_box, current_depth, ideal_depth);
            } else {
                const auto i = get_quadrant(node_box, entity_box);
                return add_node(data[node_index].children_index[i],
                                compute_Box_position_size(node_box, i),
                                entity, entity_box,
                                current_depth + 1, ideal_depth);
            }
            return false;
        }


        void split(const uint32_t node_index, const AABB_centroid<Point_type> &node_box) {
            assert(isLeaf(node_index) && "Only leaves can be split");
            for (auto &child: data[node_index].children_index) {
                assert(data.size() < max_quadtree_node);
                child = data.size();
                data.emplace_back();
            }
        }


        void query(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                   const AABB_centroid<Point_type> &check_box, std::vector<entt::entity> &values) const {
            for (const auto &value: data[node_index].entities) {
                if (value != entt::null) {
                    const AABB_centroid<Point_type> entity_box{get_AABB_centroid_(value), get_AABB_radius_(value)};
                    if (intersect(entity_box, check_box))
                        values.push_back(value);
                }
            }
            if (!isLeaf(node_index)) {
                for (auto i = 0; i < 4; ++i) {
                    const auto child_box = compute_Box_position_size(node_box, static_cast<sub_AABB>(i));
                    if (intersect(check_box, child_box))
                        query(data[node_index].children_index[i], child_box, check_box, values);
                }
            }
        }
    };
}


#endif //HELLO_MAC_ENTITY_QUADTREE_H
