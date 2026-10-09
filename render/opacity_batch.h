//
// Created by 潘鑫 on 2026/9/22.
//

#ifndef HELLO_MAC_OPACITY_BATCH_H
#define HELLO_MAC_OPACITY_BATCH_H
#include "AABB_box.h"
#include "Geometry_data.h"
#include "render_common/GPU_frustum_cull.h"
#include "load_gltf_model.h"
#include "render_mesh.h"
#include "shader_component.h"
#include "transform_component.h"
#include "utils/StructureOfArrays.h"


class VCB;

class opacity_batch {
public:
    using RenderableSoa = utils::StructureOfArrays<
        VKR_Primitive,    //
        Local_Space_AABB, //
        World_Space_AABB, //
        Transform_Matrix, //
        uint32_t,         // material_index 这里是索引,那么就应该去拿全局的 material
        entt::entity,     //
        VKR_Primitive,    //
        VKR_Primitive     //
    >;

    // instance 就变成了  SOA 的起始位置,   Transform 就需要去拿 entt 的 偏移了
    enum {
        SOA_Primitive,              //
        SOA_Local_Space_AABB,       //
        SOA_World_Space_AABB,       //
        SOA_Transform_Matrix,       //
        SOA_material_index,         //
        SOA_entity,                 //
        SOA_frustum_culling,        //
        SOA_shadow_frustum_culling, //   
    };

    const RenderableSoa &get_renderable_SOA_data() const {
        return mRenderableData_;
    }

    const Mesh_data &get_mesh_data() const {
        return mesh_data_;
    }

    const VKR_buffer_ptr &get_SOA_buffer() const {
        return SOA_buffer_;
    }


    void check_and_update(sets_map &sets_map_in_for,
                          std::map<std::string, Update_descriptor_binding> &update_descriptor_write);


    void frustum_cull(VCB &vcb, const std::shared_ptr<vk_shader_data> &compute_shader,
                      const FrustumPlanes &frustum_planes,
                      const std::array<FrustumPlanes, 4> &light_frustum_planes,
                      const VKR_image_ptr &depth) {
    }


    void draw_depth_batch(VCB &vcb, const std::shared_ptr<vk_shader_data> &shader_data_ref);

    void draw_batch(VCB &vcb);

    void draw_shadow(VCB &vcb);


    void destroy() {
        SOA_buffer_         = nullptr;
        mesh_data_.vertices = nullptr;
        mesh_data_.indices  = nullptr;
        mesh_data_          = {};
    }

private:
    VKR_buffer_ptr SOA_buffer_; // 这里存储同一批物体的参数
    // 不能直接在原本的位置上修改
    RenderableSoa mRenderableData_;
    Mesh_data mesh_data_; // 这里存储同一批物体的mesh
    Geometry_data bindless_Geometry_data_;
};


#endif //HELLO_MAC_OPACITY_BATCH_H
