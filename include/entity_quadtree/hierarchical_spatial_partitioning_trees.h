//
// Created by 潘鑫 on 2026/6/12.
//

#ifndef HELLO_MAC_HIERARCHICAL_SPATIAL_PARTITIONING_TREES_H
#define HELLO_MAC_HIERARCHICAL_SPATIAL_PARTITIONING_TREES_H

#include <vector>
#include <entt/entt.hpp>


template<typename T, std::size_t N>
constexpr std::array<T, N> make_filled_array(const T &value) {
    std::array<T, N> arr{};
    for (std::size_t i = 0; i < N; ++i) {
        arr[i] = value;
    }
    return arr;
}

class Spatial_Tree_Node_Vector {
public:
    std::vector<entt::entity> entities;
    std::vector<uint32_t> children_index;

    [[nodiscard]] bool is_leaf() const {
        return children_index.empty();
    }

    bool add_entity(const entt::entity entity) {
        entities.emplace_back(entity);
        return true;
    }

    bool get_all_entity(std::vector<entt::entity> &result) const {
        for (auto &entity_ref: entities) {
            result.emplace_back(entity_ref);
        }
        return true;
    }

    /**
     * 移除的函数不需要做任何的更改
     * @param entity
     * @return
     */
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

    [[nodiscard]] bool empty() const {
        return entities.empty();
    }

    [[nodiscard]] size_t size() const {
        return entities.size();
    }
};


template<std::size_t N, std::size_t M>
class Spatial_Tree_Pool_Element {
public:
    std::array<uint32_t, N> children_index = make_filled_array<uint32_t, N>(std::numeric_limits<uint32_t>::max());
    std::array<entt::entity, M> entities   = make_filled_array<entt::entity, M>(entt::null);
    // std::vector<entt::entity> vector_entities;
    uint32_t next_entities_index = std::numeric_limits<uint32_t>::max();

    [[nodiscard]] bool is_leaf() const {
        if (children_index == make_filled_array<uint32_t, N>(std::numeric_limits<uint32_t>::max()))
            return true;
        return false;
    }


    void clean() {
        for (auto &entity_ref: entities) {
            entity_ref = entt::null;
        }
        for (auto &child: children_index) {
            child = std::numeric_limits<uint32_t>::max();
        }
        next_entities_index = std::numeric_limits<uint32_t>::max();
    }

    bool add_entity(const entt::entity entity) {
        for (auto &entity_ref: entities) {
            if (entity_ref == entt::null) {
                entity_ref = entity;
                return true;
            }
        }
        return false;
    }

    bool get_all_entity(std::vector<entt::entity> &result) const {
        for (auto &entity_ref: entities) {
            if (entity_ref == entt::null) {
                result.emplace_back(entity_ref);
            }
        }
        return true;
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


    [[nodiscard]] bool empty() const {
        if (entities == make_filled_array<entt::entity, M>(entt::null))
            return true;
        return false;
    }

    [[nodiscard]] size_t size() const {
        size_t result = 0;
        for (auto &entity_ref: entities) {
            if (entity_ref != entt::null) {
                result++;
            }
        }
        return result;
    }
};

template<std::size_t N, std::size_t M>
class Spatial_Tree_Pool_Tree {


};


#endif //HELLO_MAC_HIERARCHICAL_SPATIAL_PARTITIONING_TREES_H
