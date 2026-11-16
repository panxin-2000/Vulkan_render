//
// Created by 潘鑫 on 2026/9/22.
//

#ifndef HELLO_MAC_OPACITY_BATCH_H
#define HELLO_MAC_OPACITY_BATCH_H
#include "AABB_box.h"
#include "Geometry_data.h"
#include "mesh_component.h"
#include "../render/render_common/GPU_frustum_cull.h"
#include "../render/render_component/primitive_component.h"
#include "../render/render_component/shader_component.h"
#include "transform_component.h"
#include "utils/StructureOfArrays.h"


class VCB;

class opaque_batch {
public:
    using RenderableSoa = utils::StructureOfArrays<
        VKR_Primitive,    // 需要绘制的 单个 primitive 的顶点和索引
        Local_Space_AABB, // 模型的包围盒
        World_Space_AABB, // 模型在世界空间的包围盒， 可以交由 GPU 计算 
        Transform_Matrix, // 模型的 变化矩阵
        uint32_t,         // material_index 这里是索引,那么就应该去拿全局的 material
        entt::entity,     // 顶点组成的三角形的 实体
        VKR_Primitive,    //  一个是 太阳光 照射到需要绘制的阴影
        VKR_Primitive     //  一个是 摄像机 能录制到的物体
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

    [[nodiscard]] const RenderableSoa &get_renderable_SOA_data() const {
        return mRenderableData_;
    }

    [[nodiscard]] const Mesh_data &get_mesh_data() const {
        return mesh_data_;
    }

    [[nodiscard]] const VKR_buffer_ptr &get_SOA_buffer() const {
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
        SOA_buffer_          = nullptr;
        mesh_data_.vertices_ = nullptr;
        mesh_data_.indices_  = nullptr;
        mesh_data_           = {};
    }

private:
    VKR_buffer_ptr SOA_buffer_; // 这里存储同一批物体的参数
    // 不能直接在原本的位置上修改
    RenderableSoa mRenderableData_;
    Mesh_data mesh_data_; // 这里存储同一批物体的mesh
    Geometry_data bindless_Geometry_data_;

    // 这里需要什么? 由 新更新的 bindless_Geometry_data_ 计算出新的  std::vector<VKR_Primitive>
    // 然后比较新旧的区别, 之后,  知道了区别, 然后就是执行 copy 了,
    // 理论上两个是能够对比的,但是实际上还是有问题, 比如一个新插入的, 是直接写入到哪里呢?
    // 找一个最后的, 先写到最后面? 之后呢?
    // 另一个,如果中间某个删除呢? 那块位置是空闲呢? 还是 之后的内存整体进行一次搬运呢?
};


#endif //HELLO_MAC_OPACITY_BATCH_H
