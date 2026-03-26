// //
// // Created by 潘鑫 on 2026/3/27.
// //
// #include "vulkan_texture_bindless.h"
// #include "vulkan_update_descriptor.h"
// #include <map>
//
//
// uint32_t free_bindless_uniform_sampler2D(const std::string &name, ) {
//     const auto world_entity = world_scene_root::get();
//     auto &bindless          = Logic_entt().get_or_emplace<bindless_uniform_sampler2D>(world_entity);
//     free_bindless_uniform_sampler2D(name, bindless);
// }
//
// uint32_t free_bindless_uniform_sampler2D(const std::string &name, bindless_uniform_sampler2D &bindless) {
//     const auto it_offset = bindless.bindings.find(name);
//     if (it_offset != bindless.bindings.end()) {
//         bindless.freeSlots.emplace(it_offset->second.first);
//         bindless.bindings.erase(it_offset);
//     } else {
//     }
// }
//
//
// uint32_t add_bindless_uniform_sampler2D(const std::string &name,
//                                         std::optional<Texture_parameter> &update,
//                                         bindless_uniform_sampler2D &bindless) {
// }
//
//
// uint32_t add_bindless_uniform_sampler2D(const std::string &name,
//                                         std::optional<Texture_parameter> &update) {
//     const auto world_entity = world_scene_root::get();
//     auto &bindless          = Logic_entt().get_or_emplace<bindless_uniform_sampler2D>(world_entity);
//     auto return_value       = bindless.bindings.size() + bindless.freeSlots.size(); // free为空时，在最大值处更新
//     if (!bindless.freeSlots.empty()) {
//         // 不为空时，拿取 队列中的 第一个被释放的 slot
//         return_value = bindless.freeSlots.front();
//         bindless.freeSlots.pop();
//     }
//     if (const auto shader_temp = Logic_entt().try_get<VKR_shader_paths>(world_entity)) {
//         if (!Logic_entt().all_of<std::shared_ptr<vk_shader_data> >(world_entity)) {
//             Logic_entt().emplace<std::shared_ptr<vk_shader_data> >(world_entity, VKR_shader_init(*shader_temp));
//         }
//         const auto &shader_data = Logic_entt().get<std::shared_ptr<vk_shader_data> >(world_entity);
//         auto &parameter         = Logic_entt().get_or_emplace<Parameter_used>(world_entity);
//
//         for (auto const &[set_value, bindings_map]: shader_data->bindless_sets_bindings) {
//             for (const auto &[binding_value, info]: bindings_map) {
//                 if (info.binding_name == "bindless_samplerColorMap") {
//                     Update_descriptor_binding temp                  = {};
//                     temp.binding_name                               = "bindless_samplerColorMap";
//                     temp.resource_type                              = "uniform sampler2D";
//                     temp.dstSet                                     = set_value; // get 不用 get 了
//                     temp.descriptor_write_binding.sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//                     temp.descriptor_write_binding.dstBinding        = binding_value;
//                     temp.descriptor_write_binding.dstArrayElement   = bindless.bindings.size();
//                     temp.descriptor_write_binding.descriptorCount   = 1;
//                     temp.descriptor_write_binding.pBufferInfo       = nullptr;
//                     temp.descriptor_write_binding.pImageInfo        = nullptr;
//                     temp.descriptor_write_binding.pTexelBufferView  = nullptr;
//                     temp.descriptor_write_binding.descriptorType    = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//                     temp.texture_info                               = {true, update.value()};
//                     parameter.update_bindless_descriptor_sets[name] = temp;
//                     // 这个时候需要做什么呢？ 添加一个更新的函数，这是记录了需要更新的内容，还没有真正更新
//                     bindless.bindings[name] = {return_value, temp};
//                 }
//             }
//         }
//     }
//     add_bindless_update_tag();
//     return return_value;
// }
