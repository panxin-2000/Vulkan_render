//
// Created by 潘鑫 on 2026/3/16.
//


#define NANOVDB_USE_OPENVDB
#include <openvdb/openvdb.h>

#include <openvdb/tools/LevelSetSphere.h> // replace with your own dependencies for generating the OpenVDB grid
#include <fstream>
#include <nanovdb/tools/CreateNanoGrid.h> // converter from OpenVDB to NanoVDB (includes NanoVDB.h and GridManager.h)
#include <nanovdb/io/IO.h>
#include <nanovdb/math/SampleFromVoxels.h>

#include "name_component.h"
#include "scene_component.h"
#include "shader_component.h"
#include "update_push_constants_data.h"
#include <Eigen/Dense>

void copy_nvdb1_to_gpu_memory(entt::entity entity, const std::string &name,
                              const nanovdb::GridHandle<nanovdb::HostBuffer> &handle) {
    if (!handle.empty()) {
        const auto ptr  = handle.data();
        const auto size = handle.bufferSize();
        auto buffer     = copy_data_to_gpu_memory(ptr, size);
        set_render_parameter(entity, "nanovdb_buffer", buffer);
        // set_render_parameter(entity, "nanovdb_size", size);
    }
}

void nanovdb_handle_add_box(const entt::entity entity,
                            const nanovdb::GridHandle<nanovdb::HostBuffer> &handle) {
    if (const nanovdb::GridMetaData *meta = handle.gridMetaData()) {
        // 直接获取体素索引空间的包围盒 (nanovdb::BBox<nanovdb::Coord>)
        auto indexBBox              = meta->indexBBox();
        nanovdb::Coord minCoord     = indexBBox.min();
        nanovdb::Coord maxCoord     = indexBBox.max();
        nanovdb::Vec3d voxelSize    = meta->voxelSize();
        const nanovdb::Map &gridMap = meta->map();
        auto matrix_3x3             = gridMap.mMatF;
        auto translation            = gridMap.mVecF;
        add_box_data(entity,
                     minCoord.x() + gridMap.mVecF[0],
                     minCoord.y() + gridMap.mVecF[1],
                     minCoord.z() + gridMap.mVecF[2],
                     maxCoord.x() + gridMap.mVecF[0],
                     maxCoord.y() + gridMap.mVecF[1],
                     maxCoord.z() + gridMap.mVecF[2]);
    }
}


void add_nanovdb_to_gpu(const entt::entity entity,
                        const std::string &file_name,
                        const Eigen::Vector3f offset,
                        const Eigen::Quaternionf &rotate) {
    const float angle = -90.0f * M_PI / 180.0f; // 或者直接使用 1.5707963f

    Eigen::Quaternionf rotate1 = Eigen::Quaternionf(Eigen::AngleAxisf(angle, Eigen::Vector3f::UnitX()));

    std::filesystem::path filePath = file_name;
    std::string ext                = filePath.extension().string();
    if (ext == ".vdb") {
        openvdb::initialize();
        openvdb::io::File file(file_name);
        file.open();

        // 名字包含 density、fog、smoke、temperature、flames、heat  直接当做 Fog Volume（雾气/发光体积）处理。
        // 名字包含 surface、sdf、collision                        直接当做 Level Set（表面/符号距离场）处理。
        // 名字包含 vel、velocity、force                             通常是 Vector 向量场。

        openvdb::GridPtrVec vdbGrids;
        for (auto iter = file.beginName(); iter != file.endName(); ++iter) {
            openvdb::GridBase::Ptr grid           = file.readGrid(iter.gridName());
            std::string valueType                 = grid->valueType();
            std::string gridClass                 = openvdb::GridBase::gridClassToString(grid->getGridClass());
            const openvdb::math::Transform &xform = grid->transform();
            double voxelSize                      = xform.voxelSize()[0];                        // 获取体素的物理尺寸
            openvdb::Vec3d mapIndex               = xform.indexToWorld(openvdb::Coord(1, 2, 3)); // 体素坐标转世界坐标

            // 4. 活动体素总数 (Active Voxels)
            openvdb::Index64 activeVoxels = grid->activeVoxelCount();

            // 5. 空间包围盒 (Bounding Box)
            auto bbox = grid->evalActiveVoxelDim(); // 索引空间包围盒

            vdbGrids.push_back(grid);
            std::cout << "Reading grid: " << iter.gridName() << std::endl;
        }

        // 遍历所有 Grid 名称
        for (auto iter = file.beginName(); iter != file.endName(); ++iter) {
            auto vdbGrid = file.readGrid(iter.gridName());

            if (vdbGrid) {
                std::cout << "Converting grid: " << iter.gridName() << std::endl;
                // 使用新版 createNanoGrid 函数

                auto baseGrid = file.readGrid(iter.gridName());

                if (baseGrid->isType<openvdb::FloatGrid>()) {
                    auto grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
                    nanovdb::GridHandle<nanovdb::HostBuffer> handle = nanovdb::tools::openToNanoVDB(grid);
                    copy_nvdb1_to_gpu_memory(entity, " name_todo", handle);

                    nanovdb_handle_add_box(entity, handle);
                    if (const nanovdb::GridMetaData *meta = handle.gridMetaData()) {
                        // 直接获取体素索引空间的包围盒 (nanovdb::BBox<nanovdb::Coord>)
                        nanovdb::Vec3d voxelSize    = meta->voxelSize(); //
                        const nanovdb::Map &gridMap = meta->map();
                        auto matrix_3x3             = gridMap.mMatF;
                        auto translation            = gridMap.mVecF;
                        Point_3 offset_voxel{translation[0], translation[1], translation[2]};
                        // 需要将 matrix_3x3 转化为 Eigen::Quaternionf ,  之后呢? 需要两个四元数 相乘,然后设置到最后的结果
                        Eigen::Matrix3f mat{
                            {gridMap.mMatF[0], gridMap.mMatF[1], gridMap.mMatF[2]},
                            {gridMap.mMatF[3], gridMap.mMatF[4], gridMap.mMatF[5]},
                            {gridMap.mMatF[6], gridMap.mMatF[7], gridMap.mMatF[8]}
                        };
                        Eigen::Quaternionf quaternion(mat);
                        quaternion.normalize();
                        Logic_entt().emplace_or_replace<Transform>(entity, offset, rotate1 * quaternion);
                    }
                } else if (baseGrid->isType<openvdb::Vec3fGrid>()) {
                    auto grid   = openvdb::gridPtrCast<openvdb::Vec3fGrid>(baseGrid);
                    auto handle = nanovdb::tools::openToNanoVDB(grid);
                    copy_nvdb1_to_gpu_memory(entity, " name_todo", handle);
                }
            }
        }
        // nanovdb::io::writeGrids(stream, handles)
        // 之前一直是写的这个,但是它是有问题的,它的默认输出是 NVDB2
        // 但是如果想传递给GPU,那么应该是 NVDB1
        file.close();
    } else if (ext == ".nvdb") {
        auto handles = nanovdb::io::readGrids(file_name);
        for (auto &handle: handles) {
            std::string gridName         = handle.gridMetaData()->shortGridName();
            nanovdb::GridClass gridClass = handle.gridMetaData()->gridClass();
            copy_nvdb1_to_gpu_memory(entity, " name_todo", handle);
        }
    }
}


entt::entity add_volume_pass(const std::string &name,
                             const Eigen::Vector3f offset     = Eigen::Vector3f(500, 200, 0),
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity()) {
    entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);


    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/render_nanovdb_different_density.frag.spv",
               "", "");

    add_nanovdb_to_gpu(entity, name, offset, rotate);
    // 更新物体的模型矩阵

    world_root_add_child(entity);
    logic_update_add_tag<volume_pass_tag>(entity);

    // transform 需要在之前的 add_nanovdb_to_gpu 中设置
    const auto transform   = Logic_entt().get<Transform>(entity);
    const auto modelMatrix = get_model_matrix(transform);
    set_render_parameter(entity, "model_4x4", modelMatrix);

    Logic_entt().emplace<Name_component>(entity, "nanovdb_volume");
    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    auto primitives = create_primitives(entity);
    logic_update_proxy(entity, primitives);
    return entity;
}
