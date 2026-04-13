//
// Created by 潘鑫 on 2026/3/10.
//

#include "PBR_component.h"

#include "global_singleton.h"
#include "shader_component.h"

void set_PBR_base_color(const entt::entity entity, Color baseColorFactor) {
    auto material             = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.baseColorFactor_ = baseColorFactor;
    set_render_parameter(entity, "object_material", material);
}

void set_PBR_Emissive_color(const entt::entity entity, Color EmissiveFactor) {
    auto material            = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.emissiveFactor_ = EmissiveFactor;
    set_render_parameter(entity, "object_material", material);
}

void set_PBR_metallic_roughness_occlusion(const entt::entity entity, float metallic, float roughness, float occlusion) {
    auto material                = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.metallicFactor_     = metallic;
    material.roughnessFactor_    = roughness;
    material.occlusion_strength_ = occlusion;
    set_render_parameter(entity, "object_material", material);
}

void set_baseColor_Texture_index(const entt::entity entity, uint32_t index) {
    auto material             = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.baseColorTexture = index;
    set_render_parameter(entity, "object_material", material);
}

void set_normal_Texture_index(const entt::entity entity, uint32_t index) {
    auto material          = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.normalTexture = index;
    set_render_parameter(entity, "object_material", material);
}

void set_emissive_Texture_index(const entt::entity entity, uint32_t index) {
    auto material            = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.emissiveTexture = index;
    set_render_parameter(entity, "object_material", material);
}

void set_ORM_Texture_index(const entt::entity entity, uint32_t index) {
    auto material        = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.ORM_Texture = index;
    set_render_parameter(entity, "object_material", material);
}
