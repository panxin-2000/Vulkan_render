//
// Created by 潘鑫 on 2026/8/13.
//

#include "../engine.h"
#include "render_proxy.h"
#include "../vulkan_code/vertex_and_buffer_index.h"
#include "name_component.h"
#include "VCB_vulkan_command_buffer.h"

void VCB::build_compute_dispatch(entt::entity entity) {
    // 下面一行估计还是有问题
    const auto &shader_data_ref = Render_entt().get<shader_data>(entity);

    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, shader_data_ref->pipeline_t);
    bind_Proxy_descriptor_sets(entity, shader_data_ref->pipeline_layout, VK_PIPELINE_BIND_POINT_COMPUTE);

    if (const auto group_count = Render_entt().try_get<compute_group_count>(entity)) {
        vkCmdDispatch(command_buffer_, group_count->X, group_count->Y, group_count->Z);
    }
    // layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in; // 工作组大小
    // 上面是什么内容呢？ 第一个需要理解的是，wave,  local_size 的 总数必须是 wave (32或64) 的整数倍
    // local_size 总是是分配在一个 CU 中
    // 单个工作组（Workgroup / Group）中的所有线程，绝对会被分配在同一个硬件计算单元中执行，无法跨单元拆分
    //    共享内存 (shared / Local Data Share, LDS)：同一个 Group 内的线程可以通过高速片上缓存直接交换数据。
    //    组内同步 (barrier() / 屏障)：可以强制让组内所有线程暂停，直到大家都走到这一步。
    // 上面的两个特征限制了 单个 CU 中的数量
    // 如果需要更多的线程数量，那么就需要在 group_count 中申请更多的数量了
    //    layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
    //    vkCmdDispatch(cmd, 32, 32, 1);
    //    512 * 512 分辨率的 图片，gl_GlobalInvocationID.x 对应行坐标，gl_GlobalInvocationID.y 对应列坐标
    //  只能保证 16 * 16 个 的一个 局部图片在一个 CU 中
    //  虽然都是可以凑到需要的线程数，但是需要考虑怎么凑才能 访问 绝对连续的显存空间，触发全速合并访问
    // }
}
