//
// Created by 潘鑫 on 2026/6/11.
//

#ifndef HELLO_MAC_ENTITY_QUADTREE_H
#define HELLO_MAC_ENTITY_QUADTREE_H


#include <array>
#include <cstddef>

#include "Box.h"
#include "global_singleton.h"
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
        std::vector<entt::entity> vector_entities;

        bool add_entity(const entt::entity entity) {
            for (auto &entity_ref: entities) {
                if (entity_ref == entt::null) {
                    entity_ref = entity;
                    return true;
                }
            }
            return false;
        }

        bool add_entity_with_max_depth(const entt::entity entity) {
            for (auto &entity_ref: entities) {
                if (entity_ref == entt::null) {
                    entity_ref = entity;
                    return true;
                }
            }
            vector_entities.emplace_back(entity);
            return true;
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
                          const get_radius &get_AABB_radius
        ) : mRootBox_position_size(box), get_AABB_centroid_(get_AABB_centroid), get_AABB_radius_(get_AABB_radius) {
            data.reserve(max_quadtree_node);
            data.emplace_back();
        }

        const uint32_t max_quadtree_node = 100; // 最多允许的四叉树 结点数量
        std::vector<Quadtree_node> data;
        AABB_centroid<Point_type> mRootBox_position_size;
        uint32_t mRoot = 0;

        bool add_entity(const entt::entity entity, const AABB_centroid<Point_type> &entity_box) {
            return add_node(0, mRootBox_position_size, entity, entity_box, 0);
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

        /**
        *
        * @param entity_box 被检索的包围盒
        * @param node_box 需要查找的包围
        * @return 在被检索的包围盒的 上下左右的哪个位置
        */
        [[nodiscard]] sub_AABB getQuadrant(const AABB_centroid<Point_2> &node_box,
                                           const AABB_centroid<Point_2> &entity_box) const {
            if (entity_box.centroid_point_.x < node_box.centroid_point_.x) {
                if (entity_box.centroid_point_.y < node_box.centroid_point_.y)
                    return South_West;
                else if (entity_box.centroid_point_.y >= node_box.centroid_point_.y)
                    return North_West;
            } else if (entity_box.centroid_point_.x >= node_box.centroid_point_.x) {
                if (entity_box.centroid_point_.y < node_box.centroid_point_.y)
                    return South_East;
                else if (entity_box.centroid_point_.y >= node_box.centroid_point_.y)
                    return North_East;
            }
        }

    private:
        Get_centroid get_AABB_centroid_;
        get_radius get_AABB_radius_;
        static constexpr auto MaxDepth = static_cast<std::size_t>(8);


        bool add_entity(const uint32_t node_index, const entt::entity entity) {
            assert(node_index< data.size());
            return data[node_index].add_entity(entity);
        }

        bool add_entity_with_max_depth(const uint32_t node_index, const entt::entity entity) {
            assert(node_index< data.size());
            return data[node_index].add_entity_with_max_depth(entity);
        }

        [[nodiscard]] bool remove_entity(const uint32_t node_index, const entt::entity entity) const {
            // Find the value in node->values
            auto entities = data[node_index].entities;
            for (auto &value: entities) {
                if (entity == value) {
                    value = entt::null;
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] bool remove_vector_entity(const uint32_t node_index, const entt::entity entity) const {
            // Find the value in node->values
            auto entities = data[node_index].entities;
            for (auto &value: entities) {
                if (entity == value) {
                    value = entt::null;
                    return true;
                }
            }
            auto vector_entities = data[node_index].vector_entities;
            for (size_t i = 0; i < vector_entities.size(); ++i) {
                if (vector_entities[i] == entity) {
                    vector_entities[i] = vector_entities.back();
                    vector_entities.pop_back();
                    return true;
                }
            }
            return false;
        }


        bool remove_node(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                         const entt::entity entity, const AABB_centroid<Point_type> &entity_box) {
            if (!isLeaf(node_index)) {
                // Remove the value from node
                if (remove_entity(node_index, entity))
                    return true;
                const auto i = getQuadrant(node_box, entity_box);
                return remove_node(data[node_index].children_index[i],
                                   compute_Box_position_size(node_box, i), entity, entity_box);
            } else {
                return remove_vector_entity(node_index, entity);
            }
        }

        bool add_node(const uint32_t node_index, const AABB_centroid<Point_type> &node_box,
                      const entt::entity entity, const AABB_centroid<Point_type> &entity_box, const uint32_t depth) {
            if (isLeaf(node_index)) {
                if (add_entity(node_index, entity)) {
                    return true;
                } else if (depth >= MaxDepth) {
                    // 已经为最大的深度了不能再深入了
                    add_entity_with_max_depth(node_index, entity);
                } else {
                    split(node_index, node_box); // split 依旧在同一个格子中，那么 add_node 还会再分裂
                    return add_node(node_index, node_box, entity, entity_box, depth);
                }
            } else {
                const auto i = getQuadrant(node_box, entity_box);
                // Add the value in a child if the value is entirely contained in it
                return add_node(data[node_index].children_index[i],
                                compute_Box_position_size(node_box, i), entity, entity_box, depth + 1);
            }
            return false;
        }


        void split(const uint32_t node_index, const AABB_centroid<Point_type> &node_box) {
            assert(isLeaf(node_index) && "Only leaves can be split");
            // Create children

            for (auto &child: data[node_index].children_index) {
                assert(data.size() < max_quadtree_node);
                child = data.size();
                data.emplace_back();
            }
            // Assign values to children
            for (const auto &entity: data[node_index].entities) {
                auto i = getQuadrant(node_box, {get_AABB_centroid_(entity), get_AABB_radius_(entity)});
                // 这里确实是有问题需要考虑的， 如果多次集中在同一个小格子中
                // 那么之后添加还是会出现问题的
                add_entity(data[node_index].children_index[i], entity);
            }
            data[node_index].entities = make_filled_array<entt::entity, 8>(entt::null);
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
                    if (intersects(check_box, child_box))
                        query(data[node_index].children_index[i], child_box, check_box, values);
                }
            }
        }
    };
}


#endif //HELLO_MAC_ENTITY_QUADTREE_H
