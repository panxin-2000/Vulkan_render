//
// Created by 潘鑫 on 2026/3/10.
//

#include "PBR_component.h"

#include "global_singleton.h"
#include "../render_component/shader_component.h"

void set_PBR_base_color(const entt::entity entity, Color baseColorFactor) {
    auto material             = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.baseColorFactor_ = baseColorFactor;
    logic_set_render_parameter(entity, "object_material", material);
}

void set_PBR_Emissive_color(const entt::entity entity, Color EmissiveFactor) {
    auto material            = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.emissiveFactor_ = EmissiveFactor;
    logic_set_render_parameter(entity, "object_material", material);
}

void set_PBR_metallic_roughness_occlusion(const entt::entity entity, float metallic, float roughness, float occlusion) {
    auto material                = Logic_entt().get_or_emplace<PBR_component>(entity);
    material.metallicFactor_     = metallic;
    material.roughnessFactor_    = roughness;
    material.occlusion_strength_ = occlusion;
    logic_set_render_parameter(entity, "object_material", material);
}

void set_baseColor_Texture_index(const entt::entity entity, const std::optional<Texture_parameter> &texture) {
    // 还是需要 通过 engine 将 texture 写入到 bindless
    auto &engine = Engine::instance();
    engine.add_bindless_texture(texture);
    auto &material            = Logic_entt().get_or_emplace<PBR_component>(entity);
    auto &ptr                 = Logic_entt().get_or_emplace<PBR_Texture_ptr>(entity);
    ptr.baseColorTexture      = texture.value();
    material.baseColorTexture = texture.value().image->get_index();
    // 具体的 index 在这里的时候已经被更新 // 这里的颜色不对 应该是 ktx 的问题
    logic_set_render_parameter(entity, "object_material", material);
}

void set_normal_Texture_index(const entt::entity entity, const std::optional<Texture_parameter> &texture) {
    auto &engine = Engine::instance();
    engine.add_bindless_texture(texture);
    auto &material         = Logic_entt().get_or_emplace<PBR_component>(entity);
    auto &ptr              = Logic_entt().get_or_emplace<PBR_Texture_ptr>(entity);
    ptr.normalTexture      = texture.value();
    material.normalTexture = texture.value().image->get_index();
    logic_set_render_parameter(entity, "object_material", material);
}

void set_emissive_Texture_index(const entt::entity entity, const std::optional<Texture_parameter> &texture) {
    auto &engine = Engine::instance();
    engine.add_bindless_texture(texture);
    auto &material           = Logic_entt().get_or_emplace<PBR_component>(entity);
    auto &ptr                = Logic_entt().get_or_emplace<PBR_Texture_ptr>(entity);
    ptr.emissiveTexture      = texture.value();
    material.emissiveTexture = texture.value().image->get_index();;
    logic_set_render_parameter(entity, "object_material", material);
}

void set_ORM_Texture_index(const entt::entity entity, const std::optional<Texture_parameter> &texture) {
    auto &engine = Engine::instance();
    engine.add_bindless_texture(texture);
    auto &material       = Logic_entt().get_or_emplace<PBR_component>(entity);
    auto &ptr            = Logic_entt().get_or_emplace<PBR_Texture_ptr>(entity);
    ptr.ORM_Texture      = texture.value();
    material.ORM_Texture = texture.value().image->get_index();
    logic_set_render_parameter(entity, "object_material", material);
}
