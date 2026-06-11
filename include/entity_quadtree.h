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

        bool add_entity(const entt::entity entity) {
            for (auto &entity_ref: entities) {
                if (entity_ref == entt::null) {
                    entity_ref = entity;
                    return true;
                }
            }
            return false;
        }
    };

    class Quadtree {
    public:
        explicit Quadtree(const AABB_centroid<Point_2> &box) : mRootBox_position_size(box) {
            data.reserve(max_quadtree_node);
            data.emplace_back();
        }

        const uint32_t max_quadtree_node = 100; // 最多允许的四叉树 结点数量
        std::vector<Quadtree_node> data;
        AABB_centroid<Point_2> mRootBox_position_size;
        uint32_t mRoot = 0;

        bool add_entity(const entt::entity entity) {
            auto &entity_box = Logic_entt().get<AABB_centroid<Point_2> >(entity);
            return add_node(0, mRootBox_position_size, entity, entity_box);
        }

        bool remove_entity(const entt::entity entity) {
            return true;
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

        enum sub_AABB {
            invalid = -1,
            North_West,
            North_East,
            South_West,
            South_East,
        };

        /**
         *
         * @param box 大的包围盒
         * @param i 上下左右 四个 子包围盒的 索引
         * @return 子包围的大小与范围
         */
        static AABB_centroid<Point_2> compute_Box_position_size(const AABB_centroid<Point_2> &box, const sub_AABB i) {
            auto point_xy = Point_2{-1.0f * (static_cast<float>(i / 2) - 0.5f), static_cast<float>(i % 2) - 0.5f};
            return {
                box.centroid_point_ + box.direction_interval_ * point_xy, box.direction_interval_ * 0.5f, false
            };
            switch (i) {
                case North_West:
                    return {
                        box.centroid_point_ + box.direction_interval_ * Point_2{-0.5f, 0.5f},
                        box.direction_interval_ * 0.5f
                    };
                case North_East:
                    return {
                        box.centroid_point_ + box.direction_interval_ * Point_2{0.5f, 0.5f},
                        box.direction_interval_ * 0.5f
                    };
                case South_West:
                    return {
                        box.centroid_point_ + box.direction_interval_ * Point_2{-0.5f, -0.5f},
                        box.direction_interval_ * 0.5f
                    };
                case South_East:
                    return {
                        box.centroid_point_ + box.direction_interval_ * Point_2{0.5f, -0.5f},
                        box.direction_interval_ * 0.5f
                    };
                default:
                    assert(false && "Invalid child index");
                    return AABB_centroid<Point_2>();
            }
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
        bool add_entity(const uint32_t node_index, const entt::entity entity) {
            assert(node_index< data.size());
            return data[node_index].add_entity(entity);
        }

        bool add_node(const uint32_t node_index, const AABB_centroid<Point_2> &node_box,
                      const entt::entity entity, AABB_centroid<Point_2> &entity_box) {
            if (isLeaf(node_index)) {
                if (add_entity(node_index, entity)) {
                    return true;
                } else {
                    split(node_index, node_box); // split 依旧在同一个格子中，那么 add_node 还会再分裂
                    return add_node(node_index, node_box, entity, entity_box);
                }
            } else {
                const auto i = getQuadrant(node_box, entity_box);
                // Add the value in a child if the value is entirely contained in it
                return add_node(data[node_index].children_index[i],
                                compute_Box_position_size(node_box, i), entity, entity_box);
            }
            return false;
        }

        static AABB_centroid<Point_2> &get_entity_box(const entt::entity entity) {
            return Logic_entt().get<AABB_centroid<Point_2> >(entity);;
        }


        void split(const uint32_t node_index, const AABB_centroid<Point_2> &node_box) {
            assert(isLeaf(node_index) && "Only leaves can be split");
            // Create children

            for (auto &child: data[node_index].children_index) {
                assert(data.size() < max_quadtree_node);
                child = data.size();
                data.emplace_back();
            }
            // Assign values to children
            for (const auto &entity: data[node_index].entities) {
                auto entity_box = get_entity_box(entity);
                auto i          = getQuadrant(node_box, entity_box);
                // 这里确实是有问题需要考虑的， 如果多次集中在同一个小格子中
                // 那么之后添加还是会出现问题的
                add_entity(data[node_index].children_index[i], entity);
            }
            data[node_index].entities = make_filled_array<entt::entity, 8>(entt::null);
        }
    };
}


#endif //HELLO_MAC_ENTITY_QUADTREE_H
