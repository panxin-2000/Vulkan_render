//
// Created by 潘鑫 on 2026/9/24.
//

#include "../../UI/opacity_batch.h"
#include "VCB_vulkan_command_buffer.h"

void VCB::frustum_culling_SOA(Engine &engine,
                              const FrustumPlanes &frustum_planes,
                              const std::array<FrustumPlanes, 4> &light_frustum_planes,
                              const VKR_image_ptr &depth,
                              const opacity_batch &SOA_opacity) {
    if (SOA_opacity.get_renderable_SOA_data().size() > 0) {
        auto frustum_culling_buffer = VKR_buffer_offset{
            SOA_opacity.get_SOA_buffer(), SOA_opacity.get_renderable_SOA_data().capacity() * sizeof(VKR_Primitive),
            opacity_batch::RenderableSoa::getOffset(opacity_batch::SOA_frustum_culling,
                                                    SOA_opacity.get_renderable_SOA_data().capacity())
        };
        auto shadow_frustum_culling_buffer = VKR_buffer_offset{
            SOA_opacity.get_SOA_buffer(), SOA_opacity.get_renderable_SOA_data().capacity() * sizeof(VKR_Primitive),
            opacity_batch::RenderableSoa::getOffset(opacity_batch::SOA_shadow_frustum_culling,
                                                    SOA_opacity.get_renderable_SOA_data().capacity())
        };
        auto world_space_aabb_buffer = VKR_buffer_offset{
            SOA_opacity.get_SOA_buffer(), SOA_opacity.get_renderable_SOA_data().capacity() * sizeof(World_Space_AABB),
            opacity_batch::RenderableSoa::getOffset(opacity_batch::SOA_World_Space_AABB,
                                                    SOA_opacity.get_renderable_SOA_data().capacity())
        };
        auto Primitives_buffer = VKR_buffer_offset{
            SOA_opacity.get_SOA_buffer(), SOA_opacity.get_renderable_SOA_data().capacity() * sizeof(VKR_Primitive),
            opacity_batch::RenderableSoa::getOffset(opacity_batch::SOA_Primitive,
                                                    SOA_opacity.get_renderable_SOA_data().capacity())
        };


        calculate_frustum_cull(engine, frustum_planes, light_frustum_planes, depth,
                               SOA_opacity.get_renderable_SOA_data().size(),
                               frustum_culling_buffer,
                               Primitives_buffer,
                               world_space_aabb_buffer,
                               shadow_frustum_culling_buffer);
    }
}
