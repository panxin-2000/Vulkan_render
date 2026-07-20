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
#include "time_measure.h"
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
    std::vector<bool> removed;
    std::vector<std::vector<insert_data> > intersections1;

    auto quadtree = Quadtree<insert_data>(box); {
        ScopedTimer timer("quadtree with creation"); {
            ScopedTimer timer("quadtree");
            for (auto &node: nodes)
                quadtree.add(node);
            // Randomly remove some nodes
            auto generator         = std::default_random_engine();
            auto deathDistribution = std::uniform_int_distribution(0, 1);
            removed                = std::vector<bool>(nodes.size(), false);
            std::generate(std::begin(removed), std::end(removed),
                          [&generator, &deathDistribution]() { return deathDistribution(generator); });
            for (auto &node: nodes) {
                if (removed[node.id])
                    quadtree.remove(node);
            }
        }
        // Quadtree
        intersections1 = std::vector<std::vector<insert_data> >(nodes.size());
    }
    for (const auto &node: nodes) {
        if (!removed[node.id])
            intersections1[node.id] = quadtree.query(node);
    }
    // Brute force
    auto intersections2 = computeIntersections(nodes, removed);
    // Check
    // checkIntersections(intersections1[node.id], intersections2[node.id]);
    // Find all intersections
    auto intersections3 = quadtree.findAllIntersections();
    std::cout << intersections3.size() << '\n';
    std::cout << intersections2.size() << '\n';
}

TEST(entt, quadtree_same_point) {
    static entt::registry instance;
#define Logic_entt() instance

    auto get_AABB_centroid = [](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_2> >(entity).get_centroid();
    };
    auto get_AABB_radius = [](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_2> >(entity).get_radius();
    };

    auto quadtree = ECS::Spatial_Tree_Pool<Point_2>(AABB_centroid<Point_2>{
                                                        {0.5, 0.5f}, {0.5f, 0.5f}
                                                    });
    for (auto i = 0u; i < 10; ++i) {
        const entt::entity entity = Logic_entt().create();
        Logic_entt().emplace<AABB_centroid<Point_2> >(entity, AABB_centroid<Point_2>{
                                                          {0.6f, 0.6f}, {0.1f, 0.1f}
                                                      });
        quadtree.add_entity(entity, Logic_entt().get<AABB_centroid<Point_2> >(entity));
    }
    auto size = quadtree.size();
    EXPECT_EQ(size, 9);
}

TEST(entt, calculate_level) {
    EXPECT_EQ(calculate_level( 0.5,0.5 ,5), 0);
    EXPECT_EQ(calculate_level( 0.5,0.5 ,5), 0);
    EXPECT_EQ(calculate_level( 0.5,0.26 ,5), 0);
    EXPECT_EQ(calculate_level( 0.5,0.25 ,5), 0);
    EXPECT_EQ(calculate_level( 0.5,0.24 ,5), 1);
    EXPECT_EQ(calculate_level( 0.5,0.124 ,5), 2);
}

TEST(entt, quadtree_point2) {
    auto n   = 1000;
    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);

    static entt::registry instance;
#define Logic_entt() instance


    auto get_AABB_centroid = [](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_2> >(entity).get_centroid();
    };
    auto get_AABB_radius = [](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_2> >(entity).get_radius();
    };

    auto quadtree = ECS::Spatial_Tree_Pool<Point_2>(AABB_centroid<Point_2>{
                                                        {0.5, 0.5f}, {0.5f, 0.5f}
                                                    }); {
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
    // auto result = quadtree.query(AABB_centroid<Point_2>{
    //                                  {0.5f, 0.5f}, {0.3f, 0.3f}
    //                              });
    std::vector<entt::entity> entities;
    entities.emplace_back(static_cast<entt::entity>(0));
    entities.emplace_back(static_cast<entt::entity>(1));
    entities.emplace_back(static_cast<entt::entity>(2));
    entities.emplace_back(static_cast<entt::entity>(3));
    entities.emplace_back(static_cast<entt::entity>(4));

    // std::sort(result.begin(), result.end());
    std::sort(entities.begin(), entities.end());

    // EXPECT_EQ(result, entities);
    for (const entt::entity entity: entities) {
        EXPECT_EQ(true, quadtree.remove_entity(entity, Logic_entt().get<AABB_centroid<Point_2> >(entity)));
    }

    Logic_entt().clear();
}

TEST(entt, quadtree_point3) {
    // 添加实体测试时 如果可以的话，尽量每次重制实体。
    static entt::registry instance;
#define Logic_entt() instance

    auto n   = 1000;
    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);

    auto get_AABB_centroid = [](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_3> >(entity).get_centroid();
    };
    auto get_AABB_radius = [](const entt::entity entity) {
        return Logic_entt().get<AABB_centroid<Point_3> >(entity).get_radius();
    };

    auto quadtree = ECS::Spatial_Tree_Pool<Point_3>(AABB_centroid<Point_3>{
                                                        {0.5, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}
                                                    }); {
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
    // auto result = quadtree.query(AABB_centroid<Point_3>{
    // {0.5f, 0.5f, 0.1f}, {0.3f, 0.3f, 0.1f}
    // });
    std::vector<entt::entity> entities;
    entities.emplace_back(static_cast<entt::entity>(0));
    entities.emplace_back(static_cast<entt::entity>(1));
    entities.emplace_back(static_cast<entt::entity>(2));
    entities.emplace_back(static_cast<entt::entity>(3));
    entities.emplace_back(static_cast<entt::entity>(4));

    // std::sort(result.begin(), result.end());
    std::sort(entities.begin(), entities.end());

    // EXPECT_EQ(result, entities);
    Logic_entt().clear();
}

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include "orthotree.h"


TEST(template_octree, dfg) {
    using namespace OrthoTree;

    // Example #1: Octree for points
    // Example #1: Octree for points
    {
        auto constexpr points = std::array{Point3D{0, 0, 0}, Point3D{1, 1, 1}, Point3D{2, 2, 2}};
        auto const octree     = OctreePointM(points, 3 /*max depth*/);

        auto const searchBox = BoundingBox3D{{0.5, 0.5, 0.5}, {2.5, 2.5, 2.5}};
        auto const pointIDs  = octree.RangeSearch(searchBox); //: { 1, 2 }

        auto neighborNo    = 2;
        auto pointIDsByKNN = octree.GetNearestNeighbors(Point3D{1.1, 1.1, 1.1}
                                                      , neighborNo
                                                       ); //: { 1, 2 }
    }
    // Example #2: Quadtree for bounding boxes
    {
        auto boxes = std::vector
        {
            BoundingBox2D{{0.0, 0.0}, {1.0, 1.0}},
            BoundingBox2D{{1.0, 1.0}, {2.0, 2.0}},
            BoundingBox2D{{2.0, 2.0}, {3.0, 3.0}},
            BoundingBox2D{{3.0, 3.0}, {4.0, 4.0}},
            BoundingBox2D{{1.2, 1.2}, {2.8, 2.8}}
        };

        auto quadtree = QuadtreeBoxM(boxes
                                   , 3            // max depth
                                   , std::nullopt // user-provided bounding Box for all
                                   , 1            // max element in a node
                                    );

        auto collidingIDPairs = quadtree.CollisionDetection(); //: { {1,4}, {2,4} }

        auto searchBox = BoundingBox2D{{1.0, 1.0}, {3.1, 3.1}};

        // Boxes within the range
        auto insideBoxIDs = quadtree.RangeSearch(searchBox); //: { 1, 2, 4 }

        // Overlapping Boxes with the range
        auto overlappingBoxIDs = quadtree.RangeSearch(searchBox, RangeSearchMode::Overlap);
        //: { 1, 2, 3, 4 }

        // Picked boxes
        auto pickPoint = Point2D{2.5, 2.5};
        auto pickedIDs = quadtree.PickSearch(pickPoint); //: { 2, 4 }
    }

    // OrthoTreeManaged 应该是最上层的 全部接口
    // 包括以下的内容
    //  RangeSearch
    // PickSearch
    // GetEntitiesBreadthFirst
    // GetEntitiesDepthFirst
    // TraverseEntitiesBreadthFirst
    // TraverseEntitiesDepthFirst
    // TraverseEntitiesByPriority
    // PlanePositiveSegmentation
    // FrustumCulling
    // Query
    //     ByWithin
    //     ByOverlaps
    //     ByInFrustum
    //     ByIntersecting
    //     BySatisfies
    // GetNearestNeighbors
    // CollisionDetection
    // RayIntersectedAll
    // RayIntersectedFirst
    // PlaneSearch
    // PlaneIntersection

    //
    // template<typename TOrthoTreeCore>
    // class OrthoTreeManaged

    //  using OrthoTreePointManagedND = OrthoTreeManaged<OrthoTreePointND<DIMENSION_NO, TScalar, IS_CONTIGUOUS_CONTAINER>>;

    //  OrthoTreePointND

    //  template<dim_t DIMENSION_NO, typename TScalar = BaseGeometryType, bool IS_CONTIGUOUS_CONTAINER = true, NodeGeometryStorage NODE_GEOMETRY_STORAGE = NodeGeometryStorage::MinPoint>
    //  using OrthoTreePointND = OrthoTree::OrthoTreeBase<
    //   std::conditional_t<IS_CONTIGUOUS_CONTAINER, PointEntitySpanAdapter<PointND<DIMENSION_NO, TScalar>>, PointEntityMapAdapter<PointND<DIMENSION_NO, TScalar>>>,
    //   GeneralGeometryAdapterND<DIMENSION_NO, TScalar>,
    //   PointConfiguration<NODE_GEOMETRY_STORAGE>>;

    //  template<typename TEntityAdapter, typename TGeometryAdapter, typename TConfiguration>
    //  using OrthoTreeBase = DynamicOrthoTreeBase<TEntityAdapter, TGeometryAdapter, TConfiguration>;

    // template<typename TEntityAdapter, typename TGeometryAdapter, typename TConfiguration>
    // using DynamicOrthoTreeBase = OrthoTreeQueryBase<DynamicHashOrthoTreeCore<TEntityAdapter, TGeometryAdapter, TConfiguration>>;

    // OrthoTreeQueryBase 应该是 提供了尽可能的查询办法
    // DynamicHashOrthoTreeCore   又做了什么呢？

    // template<typename TEntityAdapter, typename TGeometryAdapter, typename TConfiguration>
    // class DynamicHashOrthoTreeCore : public OrthoTreeCoreBase<TEntityAdapter, TGeometryAdapter, TConfiguration>

    // OrthoTreeCoreBase 已经是最底层的内容了

    // OrthoTreeCoreBase
    // DynamicHashOrthoTreeCore
    // OrthoTreeQueryBase
    // OrthoTreeManaged
    // 这四层类结构揭示了 attcs/OrthoTree 库的核心架构设计。
    // 它采用了非常标准的面向对象组合与继承模式，
    // 将“底层几何运算”、“内存组织、索引管理”、“空间查询算法”以及“上层业务封装”彻底解耦。
    // 为什么说是面向对象？
}
