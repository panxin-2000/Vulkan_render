//
// Created by 潘鑫 on 2026/6/11.
//


#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <nanovdb/NanoVDB.h>

#include "entity_quadtree.h"
#include "Quadtree.h"
#include "gtest/gtest.h"

namespace entt {
    enum class entity : std::uint32_t;
}

using namespace quadtree;

struct insert_data : public Box<float> {
    std::size_t id = 0;

    bool operator ==(const insert_data &right) const {
        if (id == right.id && static_cast<const Box<float> &>(*this) == static_cast<const Box<float> &>(right))
            return true;
        return false;
    }
};

std::vector<insert_data> generateRandomNodes(std::size_t n) {
    auto generator          = std::default_random_engine();
    auto originDistribution = std::uniform_real_distribution(0.0f, 1.0f);
    auto sizeDistribution   = std::uniform_real_distribution(0.0f, 0.01f);
    auto nodes              = std::vector<insert_data>(n);
    for (auto i = std::size_t(0); i < n; ++i) {
        nodes[i].left   = originDistribution(generator);
        nodes[i].top    = originDistribution(generator);
        nodes[i].width  = std::min(1.0f - nodes[i].left, sizeDistribution(generator));
        nodes[i].height = std::min(1.0f - nodes[i].top, sizeDistribution(generator));
        nodes[i].id     = i;
    }
    return nodes;
}

std::vector<std::pair<insert_data *, insert_data *> >
computeIntersections(std::vector<insert_data> &nodes, const std::vector<bool> &removed) {
    auto intersections = std::vector<std::pair<insert_data *, insert_data *> >();
    for (auto i = std::size_t(0); i < nodes.size(); ++i) {
        if (removed.size() == 0 || !removed[i]) {
            for (auto j = std::size_t(0); j < i; ++j) {
                if (removed.size() == 0 || !removed[j]) {
                    if (nodes[i].intersects(nodes[j]))
                        intersections.emplace_back(&nodes[i], &nodes[j]);
                }
            }
        }
    }
    return intersections;
}

void checkIntersections(std::vector<insert_data *> nodes1, std::vector<insert_data *> nodes2) {
    EXPECT_EQ(nodes1.size(), nodes2.size());
    std::sort(std::begin(nodes1), std::end(nodes1));
    std::sort(std::begin(nodes2), std::end(nodes2));
    for (auto i = std::size_t(0); i < nodes1.size(); ++i)
        EXPECT_EQ(nodes1[i], nodes2[i]);
}

TEST(box, quadtree) {
    auto n = std::size_t(1000);

    auto box   = Box(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(n);
    // Add nodes to quadtree
    auto quadtree     = Quadtree<insert_data>(box);
    const auto start1 = std::chrono::steady_clock::now();
    for (auto &node: nodes)
        quadtree.add(node);
    // Randomly remove some nodes
    auto generator         = std::default_random_engine();
    auto deathDistribution = std::uniform_int_distribution(0, 1);
    auto removed           = std::vector<bool>(nodes.size(), false);
    std::generate(std::begin(removed), std::end(removed),
                  [&generator, &deathDistribution]() { return deathDistribution(generator); });
    for (auto &node: nodes) {
        if (removed[node.id])
            quadtree.remove(node);
    }
    // Quadtree
    auto intersections1 = std::vector<std::vector<insert_data> >(nodes.size());
    auto start2         = std::chrono::steady_clock::now();
    for (const auto &node: nodes) {
        if (!removed[node.id])
            intersections1[node.id] = quadtree.query(node);
    }
    auto duration2 = std::chrono::steady_clock::now() - start2;
    auto duration1 = std::chrono::steady_clock::now() - start1;
    std::cout << "quadtree: " << std::chrono::duration_cast<std::chrono::microseconds>(duration2).count() << "us" <<
            '\n';
    std::cout << "quadtree with creation: " << std::chrono::duration_cast<std::chrono::microseconds>(duration1).count()
            << "us" << '\n';
    // Brute force
    auto intersections2 = computeIntersections(nodes, removed);
    // Check
    // checkIntersections(intersections1[node.id], intersections2[node.id]);
    // Find all intersections
    auto intersections3 = quadtree.findAllIntersections();
    std::cout << intersections3.size() << '\n';
    std::cout << intersections2.size() << '\n';
}

TEST(entt, quadtree_point2) {
    auto n   = 1000;
    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);

    static entt::registry instance;

    auto Logic_entt = []() {
        return std::move(instance);
    };

    auto get_AABB_centroid = [Logic_entt](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_2> >(entity).get_centroid_point();
    };
    auto get_AABB_radius = [Logic_entt](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_2> >(entity).get_radius();
    };

    auto quadtree = ECS::Quadtree<Point_2>(AABB_centroid<Point_2>{
                                               {0.5, 0.5f}, {0.5f, 0.5f}
                                           },
                                           get_AABB_centroid,
                                           get_AABB_radius); {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_2> >(entity, AABB_centroid<Point_2>{
                                                          {0.6f, 0.6f}, {0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_2> >(entity));
    } {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_2> >(entity, AABB_centroid<Point_2>{
                                                          {0.3f, 0.3f}, {0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_2> >(entity));
    } {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_2> >(entity, AABB_centroid<Point_2>{
                                                          {0.3f, 0.6f}, {0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_2> >(entity));
    } {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_2> >(entity, AABB_centroid<Point_2>{
                                                          {0.6f, 0.3f}, {0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_2> >(entity));
    } {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_2> >(entity, AABB_centroid<Point_2>{
                                                          {0.7f, 0.7f}, {0.2f, 0.2f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_2> >(entity));
    }
    auto result = quadtree.query(AABB_centroid<Point_2>{
                                     {0.5f, 0.5f}, {0.3f, 0.3f}
                                 });
    std::vector<entt::entity> entities;
    entities.emplace_back(static_cast<entt::entity>(0));
    entities.emplace_back(static_cast<entt::entity>(1));
    entities.emplace_back(static_cast<entt::entity>(2));
    entities.emplace_back(static_cast<entt::entity>(3));
    entities.emplace_back(static_cast<entt::entity>(4));
    EXPECT_EQ(result, entities);
    Logic_entt().clear();
}

TEST(entt, quadtree_point3) {
    // 添加实体测试时 如果可以的话，尽量每次重制实体。
    static entt::registry instance;
    auto Logic_entt = []() {
        return std::move(instance);
    };

    auto n   = 1000;
    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);

    auto get_AABB_centroid = [Logic_entt](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_3> >(entity).get_centroid_point();
    };
    auto get_AABB_radius = [Logic_entt](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_3> >(entity).get_radius();
    };

    auto quadtree = ECS::Quadtree<Point_3>(AABB_centroid<Point_3>{
                                               {0.5, 0.5f, 0.1f}, {0.5f, 0.5f, 0.1f}
                                           },
                                           get_AABB_centroid,
                                           get_AABB_radius); {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_3> >(entity, AABB_centroid<Point_3>{
                                                          {0.6f, 0.6f, 0.1f}, {0.1f, 0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_3> >(entity));
    } {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_3> >(entity, AABB_centroid<Point_3>{
                                                          {0.3f, 0.3f, 0.1f}, {0.1f, 0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_3> >(entity));
    } {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_3> >(entity, AABB_centroid<Point_3>{
                                                          {0.3f, 0.6f, 0.1f}, {0.1f, 0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_3> >(entity));
    } {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_3> >(entity, AABB_centroid<Point_3>{
                                                          {0.6f, 0.3f, 0.1f}, {0.1f, 0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_3> >(entity));
    } {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_3> >(entity, AABB_centroid<Point_3>{
                                                          {0.7f, 0.7f, 0.1f}, {0.2f, 0.2f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_3> >(entity));
    }
    auto result = quadtree.query(AABB_centroid<Point_3>{
                                     {0.5f, 0.5f, 0.1f}, {0.3f, 0.3f, 0.1f}
                                 });
    std::vector<entt::entity> entities;
    entities.emplace_back(static_cast<entt::entity>(0));
    entities.emplace_back(static_cast<entt::entity>(1));
    entities.emplace_back(static_cast<entt::entity>(2));
    entities.emplace_back(static_cast<entt::entity>(3));
    entities.emplace_back(static_cast<entt::entity>(4));
    EXPECT_EQ(result, entities);
    Logic_entt().clear();
}
