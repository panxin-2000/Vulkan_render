//
// Created by 潘鑫 on 2026/6/11.
//

#ifndef HELLO_MAC_QUADTREE_H
#define HELLO_MAC_QUADTREE_H


#pragma once

#include <cassert>
#include <algorithm>
#include <array>
#include <memory>
#include <type_traits>
#include <vector>
#include "Box.h"

namespace quadtree {
    template<typename T_data, typename Float =float>
    class Quadtree {
        // 强制继承 Box 的时候会 更加 优雅
        static_assert(std::is_base_of_v<Box<Float>, std::remove_pointer_t<T_data> >,
                      "T_data must publicly inherit from Box<Float>");
        static_assert(std::is_arithmetic_v<Float>);

    public:
        explicit Quadtree(const Box<Float> &box) : mRootBox_position_size(box),
                                                   mRoot(std::make_unique<Node>()) {
        }

        void add(const T_data &value) {
            add(mRoot.get(), 0, mRootBox_position_size, value);
        }

        void remove(const T_data &value) {
            remove(mRoot.get(), mRootBox_position_size, value);
        }

        std::vector<T_data> query(const Box<Float> &box) const {
            auto values = std::vector<T_data>();
            query(mRoot.get(), mRootBox_position_size, box, values);
            return values;
        }

        std::vector<std::pair<T_data, T_data> > findAllIntersections() const {
            auto intersections = std::vector<std::pair<T_data, T_data> >();
            findAllIntersections(mRoot.get(), intersections);
            return intersections;
        }

        Box<Float> getBox() const {
            return mRootBox_position_size;
        }

    private:
        static constexpr auto Threshold = static_cast<std::size_t>(16);
        static constexpr auto MaxDepth  = static_cast<std::size_t>(8);

        struct Node {
            std::array<std::unique_ptr<Node>, 4> children;
            std::vector<T_data> values;
        };

        Box<Float> mRootBox_position_size;
        std::unique_ptr<Node> mRoot; // 这里是一个 跟 结点，但是不带 具体的大小

        static bool isLeaf(const Node *node) {
            return !static_cast<bool>(node->children[0]);
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
        Box<Float> compute_Box_position_size(const Box<Float> &box, const sub_AABB i) const {
            auto origin    = box.getTopLeft();
            auto childSize = box.getSize() / static_cast<Float>(2);
            switch (i) {
                case North_West:
                    return Box<Float>(origin, childSize);
                case North_East:
                    return Box<Float>(Vector2<Float>(origin.x + childSize.x, origin.y), childSize);
                case South_West:
                    return Box<Float>(Vector2<Float>(origin.x, origin.y + childSize.y), childSize);
                case South_East:
                    return Box<Float>(origin + childSize, childSize);
                default:
                    assert(false && "Invalid child index");
                    return Box<Float>();
            }
        }

        /**
         *
         * @param nodeBox 被检索的包围盒
         * @param valueBox 需要查找的包围
         * @return 在被检索的包围盒的 上下左右的哪个位置
         */
        sub_AABB getQuadrant(const Box<Float> &nodeBox, const Box<Float> &valueBox) const {
            auto center = nodeBox.getCenter();
            // West
            if (valueBox.getRight() < center.x) {
                // North West
                if (valueBox.getBottom() < center.y)
                    return North_West;
                    // South West
                else if (valueBox.top >= center.y)
                    return South_West;
                    // Not contained in any quadrant
                else
                    return invalid;
            }
            // East
            else if (valueBox.left >= center.x) {
                // North East
                if (valueBox.getBottom() < center.y)
                    return North_East;
                    // South East
                else if (valueBox.top >= center.y)
                    return South_East;
                    // Not contained in any quadrant
                else
                    return invalid;
            }
            // Not contained in any quadrant
            else
                return invalid;
        }

        /**
         *
         * @param node 当前的node
         * @param depth 当前包围盒的深度
         * @param box 当前的范围
         * @param value 需要添加的box
         */
        void add(Node *node, std::size_t depth, const Box<Float> &box, const T_data &value) {
            assert(node != nullptr);
            assert(box.contains(mGetBox(value)));
            if (isLeaf(node)) {
                // Insert the value in this node if possible
                if (depth >= MaxDepth || node->values.size() < Threshold)
                    node->values.push_back(value);
                // Otherwise, we split and we try again
                else {
                    split(node, box);
                    add(node, depth, box, value);
                }
            } else {
                auto i = getQuadrant(box, static_cast<Box<Float>>(value));
                // Add the value in a child if the value is entirely contained in it
                if (i != invalid)
                    add(node->children[+i].get(), depth + 1, compute_Box_position_size(box, i), value);
                    // Otherwise, we add the value in the current node
                else
                    node->values.push_back(value);
            }
        }

        void split(Node *node, const Box<Float> &box) {
            assert(node != nullptr);
            assert(isLeaf(node) && "Only leaves can be split");
            // Create children
            for (auto &child: node->children)
                child = std::make_unique<Node>();
            // Assign values to children
            auto newValues = std::vector<T_data>(); // New values for this node
            for (const auto &value: node->values) {
                auto i = getQuadrant(box, (value));
                if (i != invalid)
                    node->children[static_cast<std::size_t>(i)]->values.push_back(value);
                else
                    newValues.push_back(value);
            }
            node->values = std::move(newValues);
        }

        bool remove(Node *node, const Box<Float> &box, const T_data &value) {
            assert(node != nullptr);
            assert(box.contains(mGetBox(value)));
            if (isLeaf(node)) {
                // Remove the value from node
                removeValue(node, value);
                return true;
            } else {
                // Remove the value in a child if the value is entirely contained in it
                auto i = getQuadrant(box, (value));
                if (i != invalid) {
                    if (remove(node->children[static_cast<std::size_t>(i)].get(), compute_Box_position_size(box, i),
                               value))
                        return tryMerge(node);
                }
                // Otherwise, we remove the value from the current node
                else
                    removeValue(node, value);
                return false;
            }
        }

        void removeValue(Node *node, const T_data &value) {
            // Find the value in node->values
            auto it = std::find_if(std::begin(node->values),
                                   std::end(node->values),
                                   [this, &value](const auto &rhs) { return value == rhs; });
            assert(it != std::end(node->values) && "Trying to remove a value that is not present in the node");
            // Swap with the last element and pop back
            *it = std::move(node->values.back());
            node->values.pop_back();
        }

        bool tryMerge(Node *node) {
            assert(node != nullptr);
            assert(!isLeaf(node) && "Only interior nodes can be merged");
            auto nbValues = node->values.size();
            for (const auto &child: node->children) {
                if (!isLeaf(child.get()))
                    return false;
                nbValues += child->values.size();
            }
            if (nbValues <= Threshold) {
                node->values.reserve(nbValues);
                // Merge the values of all the children
                for (const auto &child: node->children) {
                    for (const auto &value: child->values)
                        node->values.push_back(value);
                }
                // Remove the children
                for (auto &child: node->children)
                    child.reset();
                return true;
            } else
                return false;
        }

        void query(Node *node, const Box<Float> &box, const Box<Float> &queryBox, std::vector<T_data> &values) const {
            assert(node != nullptr);
            assert(queryBox.intersects(box));
            for (const auto &value: node->values) {
                if (queryBox.intersects((value)))
                    values.push_back(value);
            }
            if (!isLeaf(node)) {
                for (auto i = std::size_t(0); i < node->children.size(); ++i) {
                    auto childBox = compute_Box_position_size(box, static_cast<sub_AABB>(i));
                    if (queryBox.intersects(childBox))
                        query(node->children[i].get(), childBox, queryBox, values);
                }
            }
        }

        void findAllIntersections(Node *node, std::vector<std::pair<T_data, T_data> > &intersections) const {
            // Find intersections between values stored in this node
            // Make sure to not report the same intersection twice
            for (auto i = 0; i < node->values.size(); ++i) {
                for (auto j = 0; j < i; ++j) {
                    if ((node->values[i]).intersects((node->values[j])))
                        intersections.emplace_back(node->values[i], node->values[j]);
                }
            }
            if (!isLeaf(node)) {
                // Values in this node can intersect values in descendants
                for (const auto &child: node->children) {
                    for (const auto &value: node->values)
                        findIntersectionsInDescendants(child.get(), value, intersections);
                }
                // Find intersections in children
                for (const auto &child: node->children)
                    findAllIntersections(child.get(), intersections);
            }
        }

        void findIntersectionsInDescendants(Node *node, const T_data &value,
                                            std::vector<std::pair<T_data, T_data> > &intersections) const {
            // Test against the values stored in this node
            for (const auto &other: node->values) {
                if ((value).intersects((other)))
                    intersections.emplace_back(value, other);
            }
            // Test against values stored into descendants of this node
            if (!isLeaf(node)) {
                for (const auto &child: node->children)
                    findIntersectionsInDescendants(child.get(), value, intersections);
            }
        }
    };
}


#endif //HELLO_MAC_QUADTREE_H
