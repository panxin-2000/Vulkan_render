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
    // 0.5 0.25 0.125

    // 2. 数学公式：m = log2(root_size / max_dim) - 1
    // 现代高性能写法：利用位运算或 std::log2f
    // 这里使用标准的高效浮点数 log2 向上取整/向下取整
    const float target_level_f = std::log2f(root_size / max_dim);
    const int target_level     = static_cast<int>(std::floor(target_level_f));

    // 3. 边界限定：不能小于 0 层，也不能超过系统设定的最大深度
    if (target_level < 0) return 0;
    return std::min(static_cast<std::uint32_t>(target_level), max_depth);
}


#include <array>
#include <cstddef>
#include <utility>

#include "Box.h"
#include "entt/entt.hpp"
#include "base_geometry/base.h"
#include "base_geometry/intersect_function.h"
#include "hierarchical_spatial_partitioning_trees.h"

namespace ECS {
    /**
      * 需要确定坐标轴
      * x轴向屏幕右边方向
      * y轴向屏幕上方方向
      * z轴从屏幕指向眼睛
      * 带z的表示负的z方向
      */
    enum sub_AABB {
        invalid = -1,
        upper_left,
        upper_right,
        down_left,
        down_right,
        upper_left_z,
        upper_right_z,
        down_left_z,
        down_right_z,
        invalid_max
    };

    /**
    *
    * @param box 大的包围盒
    * @param i 上下左右 四个 子包围盒的 索引
    * @return 子包围的大小与范围
    */
    static AABB_centroid<Point_2> compute_Box_position_size(const AABB_centroid<Point_2> &box, const sub_AABB i) {
        const auto point_xy = Point_2{-1.0f * (static_cast<float>(i / 2) - 0.5f), static_cast<float>(i % 2) - 0.5f};
        return {box.centroid_point_ + box.direction_interval_ * point_xy, box.direction_interval_ * 0.5f};
    }

    static AABB_centroid<Point_3> compute_Box_position_size(const AABB_centroid<Point_3> &box, const sub_AABB i) {
        const auto point_xy = Point_3{
            -1.0f * (static_cast<float>(i / 2) - 0.5f),
            +1.0f * (static_cast<float>(i % 2) - 0.5f),
            -1.0f * (static_cast<float>(i / 4) - 0.5f)
        };
        return {box.centroid_point_ + box.direction_interval_ * point_xy, box.direction_interval_ * 0.5f};
    }

    static sub_AABB get_quadrant(const AABB_centroid<Point_3> &node_box,
                                 const AABB_centroid<Point_3> &entity_box) {
        const auto bool_x = static_cast<uint32_t>(entity_box.centroid_point_.x >= node_box.centroid_point_.x);
        const auto bool_y = static_cast<uint32_t>(entity_box.centroid_point_.y < node_box.centroid_point_.y);
        const auto bool_z = static_cast<uint32_t>(entity_box.centroid_point_.z < node_box.centroid_point_.z);
        return static_cast<sub_AABB>(bool_x + bool_y * 2 + bool_z * 4);
    }

    /**
    *
    * @param entity_box 被检索的包围盒
    * @param node_box 需要查找的包围
    * @return 在被检索的包围盒的 上下左右的哪个位置
    */
    static sub_AABB get_quadrant(const AABB_centroid<Point_2> &node_box,
                                 const AABB_centroid<Point_2> &entity_box) {
        const auto bool_x = static_cast<uint32_t>(entity_box.get_centroid().x >= node_box.get_centroid().x);
        const auto bool_y = static_cast<uint32_t>(entity_box.get_centroid().y < node_box.get_centroid().y);
        return static_cast<sub_AABB>(bool_x + bool_y * 2);
    }


    template<typename Point_type>
    class Spatial_Tree_Pool {
    public:
        explicit Spatial_Tree_Pool(const AABB_centroid<Point_type> &box) : mRootBox_position_size(box) {
            data.reserve(max_quadtree_node);
            data.emplace_back();
            node_size_ = 1;
        }

        bool add_entity(const entt::entity entity, const AABB_centroid<Point_type> &entity_box) {
            auto ideal_level = calculate_level(mRootBox_position_size.get_radius().max_value(),
                                               entity_box.get_radius().max_value(), MaxDepth);
            return add_node(0, mRootBox_position_size, entity, entity_box, 0, ideal_level);
        }


        bool remove_entity(const entt::entity entity, const AABB_centroid<Point_type> &entity_box) {
            auto ideal_level = calculate_level(mRootBox_position_size.get_radius().max_value(),
                                               entity_box.get_radius().max_value(), MaxDepth);
            return remove_node(0, mRootBox_position_size, entity, entity_box, 0, ideal_level);
        }


        [[nodiscard]] size_t size() const {
            return node_size_;
        }

    private:
        static constexpr auto BRANCH_COUNT = 1 << (sizeof(Point_type) / sizeof(float));
        static constexpr auto ENTITY_COUNT = 1 << (sizeof(Point_type) / sizeof(float));
        const uint32_t max_quadtree_node   = 100; // 最多允许的四叉树 结点数量
        std::vector<Spatial_Tree_Pool_Element<BRANCH_COUNT, ENTITY_COUNT> > data;
        std::vector<uint32_t> free_list;
        AABB_centroid<Point_type> mRootBox_position_size;
        uint32_t mRoot = 0;


        static constexpr auto MaxDepth = static_cast<std::size_t>(8);
        size_t node_size_;


        [[nodiscard]] bool is_leaf(const uint32_t node_index) const {
            return data[node_index].is_leaf();
        }

        /**
         * 终于知道为什么要有两个了，让函数按照需要选择，为了适配 函数的 const 符号
         * @param node_index
         * @return
         */
        Spatial_Tree_Pool_Element<BRANCH_COUNT, ENTITY_COUNT> &get_node(const uint32_t node_index) {
            return data[node_index];
        }

        const Spatial_Tree_Pool_Element<BRANCH_COUNT, ENTITY_COUNT> &get_node(const uint32_t node_index) const {
            return data[node_index];
        }

        [[nodiscard]] uint32_t get_next_node_index(const uint32_t node_index) const {
            return get_node(node_index).next_entities_index;
        }

        [[nodiscard]] uint32_t get_node_child_index(const uint32_t node_index, uint32_t index) const {
            assert(index >=0 && index < invalid_max);
            return get_node(node_index).children_index[index];
        }

        void set_next_node_index(const uint32_t node_index, uint32_t value) {
            get_node(node_index).next_entities_index = value;
        }

        [[nodiscard]] bool is_empty(uint32_t node_index) const;

        [[nodiscard]] bool clean(uint32_t node_index);


        /**
         * 确定性的将 entity 添加到 node_index 中
         * @param node_index
         * @param entity
         * @return
         */
        [[nodiscard]] bool add_entity(uint32_t node_index, entt::entity entity);


        /**
         * 确定性删除 entity 在 node_index 返回 true ，否则返回 false
         * @param node_index
         * @param entity
         * @return
         */
        [[nodiscard]] bool remove_entity(uint32_t node_index, entt::entity entity);

        /**
         * 获取 node_index 中的全部 entity 到 result 中
         * @param node_index
         * @param result
         * @return
         */
        bool get_all_entity(uint32_t node_index, std::vector<entt::entity> &result) const;


        bool remove_node(uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                         entt::entity entity, const AABB_centroid<Point_type> &entity_box,
                         uint32_t current_depth, uint32_t ideal_depth);

        bool add_node(uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                      entt::entity entity, const AABB_centroid<Point_type> &entity_box,
                      uint32_t current_depth, uint32_t ideal_depth = 0);

        void split(uint32_t node_index);
    };


    class Spatial_Tree_Check {


    };
}

#include "entity_quadtree_impl.h"

#endif //HELLO_MAC_ENTITY_QUADTREE_H
