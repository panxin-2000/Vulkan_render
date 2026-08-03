//
// Created by 潘鑫 on 2026/3/16.
//


#define NANOVDB_USE_OPENVDB
#include <openvdb/openvdb.h>


#include <openvdb/tools/LevelSetSphere.h> // replace with your own dependencies for generating the OpenVDB grid
#include <fstream>
#include <entt/entity/entity.hpp>
#include <nanovdb/tools/CreateNanoGrid.h> // converter from OpenVDB to NanoVDB (includes NanoVDB.h and GridManager.h)
#include <nanovdb/io/IO.h>
#include <nanovdb/math/SampleFromVoxels.h>

#include "name_component.h"
#include "scene_component.h"
#include "shader_component.h"
#include "update_push_constants_data.h"


void test(void *ptr, uint64_t size);


void copy_nvdb1_to_gpu_memory(entt::entity entity, const std::string &name,
                              const nanovdb::GridHandle<nanovdb::HostBuffer> &handle) {
    if (!handle.empty()) {
        const auto ptr  = handle.data();
        const auto size = handle.bufferSize();
        auto buffer     = copy_data_to_gpu_memory(ptr, size);
        set_render_parameter(entity, "nanovdb_buffer", buffer);

        if (const nanovdb::GridMetaData *meta = handle.gridMetaData()) {
            // 直接获取体素索引空间的包围盒 (nanovdb::BBox<nanovdb::Coord>)
            auto indexBBox              = meta->indexBBox();
            nanovdb::Coord minCoord     = indexBBox.min();
            nanovdb::Coord maxCoord     = indexBBox.max();
            nanovdb::Vec3d voxelSize    = meta->voxelSize();
            const nanovdb::Map &gridMap = meta->map();
            auto matrix_3x3             = gridMap.mMatF;
            auto translation            = gridMap.mVecF;
            // 能得到这里之后呢？ 之后 整合 为 一个旋转的矩阵
            // 提取最小体素坐标和最大体素坐标
            // 另一个问题是，单位是什么？ //
            // 这里是按照体素来的
            // 可以先绘制一下看看结果是否是 和 大小是否是对的
            add_box_data(entity,
                         minCoord.x() + gridMap.mVecF[0],
                         minCoord.y() + gridMap.mVecF[1],
                         minCoord.z() + gridMap.mVecF[2],
                         maxCoord.x() + gridMap.mVecF[0],
                         maxCoord.y() + gridMap.mVecF[1],
                         maxCoord.z() + gridMap.mVecF[2]);
            // 这里的问题， 这里导致了 volume 的颜色增加
            // Render_AABB aabb{
            //     {(float) minCoord.x(), (float) minCoord.y(), (float) minCoord.z(), 0.0f},
            //     {(float) maxCoord.x(), (float) maxCoord.y(), (float) maxCoord.z(), 0.0f}
            // };
            // set_render_parameter(entity, "nanovdb_box", aabb);
        }
        // set_render_parameter(entity, "nanovdb_size", size);
    }
}


void add_nanovdb_to_gpu(entt::entity entity) {
    auto srcGrid = openvdb::tools::createLevelSetSphere<
        openvdb::FloatGrid>(100.0f, openvdb::Vec3f(200.0f, 200.0f, 0.0f), 1.0f);
    nanovdb::GridHandle handle = nanovdb::tools::createNanoGrid(*srcGrid);

    // std::ofstream outFile("Sphere.nvdb", std::ios::binary);
    // if (!outFile) {
    // std::cerr << "无法打开文件进行写入: " << "Sphere.nvdb" << std::endl;
    // return;
    // }

    // 2. 获取 NanoVDB 数据的底层缓冲区指针和大小
    // void *dataPtr     = handle.buffer().data(); // 字节指针 [1]
    // uint64_t dataSize = handle.buffer().size(); // 字节大小 [1, 2]

    // 3. 将整个连续内存块写入文件
    // outFile.write(reinterpret_cast<const char *>(dataPtr), dataSize);

    // 4. 关闭文件
    // outFile.close();
    // return;

    copy_nvdb1_to_gpu_memory(entity, "nanovdb_buffer", handle);
}


void add_nanovdb_to_gpu(const entt::entity entity, const std::string &file_name) {
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


void convert(const std::string &filename) {
    try {
        // Create an OpenVDB grid of a sphere at the origin with radius 100 and voxel size 1.
        auto srcGrid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(100.0f, openvdb::Vec3f(0.0f), 1.0f);
        nanovdb::GridHandle handle = nanovdb::tools::createNanoGrid(*srcGrid);
        // 一个 nanovdb::GridHandle 可以包含多个网格 如密度场、温度场、速度场等

        auto ptr  = handle.data();
        auto size = handle.bufferSize();


        test(ptr, size);


        auto gridCount = handle.gridCount();
        for (uint32_t i = 0; i < gridCount; ++i) {
            auto gridType = handle.gridType(i);
            if (gridType == nanovdb::GridType::Float) {
                const auto nanoGrid = handle.grid<float>(i);

                nanovdb::CoordBBox bbox = nanoGrid->indexBBox();
                nanovdb::Coord minCoord = bbox.min();
                nanovdb::Coord maxCoord = bbox.max();

                auto name      = nanoGrid->gridName();
                auto gridClass = nanoGrid->gridClass();
                auto acc       = nanoGrid->getAccessor();
                auto &tree     = nanoGrid->tree(); // 自定义的树遍历（而不是简单的坐标查询），必须通过 tree
                // 3. 坐标读取 ，很少使用  传递的参数是3个 int 值
                nanovdb::Coord ijk(105, 205, 305);
                float value = acc.getValue(ijk);
                nanoGrid->tree().getValue(nanovdb::Coord(99, 0, 0));
                nanovdb::Vec3d worldPos(1.5, 2.0, 3.5);
                nanovdb::Vec3d indexPos = nanoGrid->worldToIndex(worldPos);
                nanovdb::Coord ijk_2    = nanovdb::Coord::Floor(indexPos);
                float value_2           = acc.getValue(ijk_2);


                auto smp = nanovdb::math::createSampler<1>(acc);
                // 创建了 sample ,之后呢？
                float value_3 = smp(indexPos); // 这里不懂， nanoGrid->worldToIndex(worldPos)
                // 有一个设计的问题
                // Grid/Tree/Accessor: 只负责处理索引空间（整数或连续的浮点索引）。
                // Map/Transform: 负责定义这个网格在世界中如何旋转、平移或缩放。
                // Sampler: 它的工作是根据给定的浮点索引，在相邻的 8 个体素之间进行插值计算。它不关心世界坐标的具体单位（米、厘米等）。
            }
        }


        // Convert from OpenVDB to NanoVDB and return a shared pointer to a GridHandle.
        const auto nanoGrid = handle.grid<float>(); // Get a (raw) pointer to the NanoVDB grid form the GridManager.
        if (!nanoGrid)
            throw std::runtime_error("GridHandle does not contain a grid with value type float");


        // 5. 基础查询测试 (类似于之前 C 语言版本的采样)
        auto accessor = nanoGrid->getAccessor();

        nanovdb::Coord ijk(0, 0, 0);
        float val = accessor.getValue(ijk);


        nanovdb::io::writeGrid("sphere2.nvdb", handle);
        // Write the NanoVDB grid to file and throw if writing fails
    } catch (const std::exception &e) {
        std::cerr << "An exception occurred: \"" << e.what() << "\"" << std::endl;
    }
}


void base_start_function() {
    // 1. 初始化 OpenVDB 库
    // 任何使用 OpenVDB 的程序都必须先调用此函数
    openvdb::initialize();

    convert("");
    // 2. 创建一个浮点网格（FloatGrid）
    // 初始值为 0.0，网格名称为 "MyGrid"
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(0.0);
    grid->setName("MyGrid");

    // 3. 获取网格的访问器（Accessor）
    // 访问器通过缓存机制提供快速的体素读写
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();

    // 4. 在指定坐标设置体素值
    // 这里我们将坐标 (10, 20, 30) 的体素值设置为 1.0
    openvdb::Coord xyz(10, 20, 30);
    accessor.setValue(xyz, 1.0);

    std::cout << "Voxel at " << xyz << " is " << accessor.getValue(xyz) << std::endl;

    // 5. 将网格保存到 .vdb 文件
    // 创建文件对象并将网格存入容器
    openvdb::io::File file("my_grid.vdb");
    openvdb::GridPtrVec grids;
    grids.push_back(grid);

    // 写入文件并关闭
    file.write(grids);
    file.close();

    std::cout << "Successfully saved grid to my_grid.vdb" << std::endl;
}



void add_nanovdb_to_gpu(entt::entity entity);

void add_nanovdb_to_gpu(const entt::entity entity, const std::string &file_name);


inline entt::entity add_volume_pass(const std::string &name,
                                    const Point_3 offset             = Point_3(500, 200, 0),
                                    const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity()) {
    entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);


    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/render_nanovdb_different_density.frag.spv",
               "", "");

    add_nanovdb_to_gpu(entity, name);
    // 更新物体的模型矩阵


    world_root_add_child(entity);
    logic_update_add_tag<volume_pass_tag>(entity);
    const auto transform   = Logic_entt().emplace<Transform>(entity, offset, rotate);
    const auto modelMatrix = get_model_matrix(transform);
    set_render_parameter(entity, "model_4x4", modelMatrix);
    Logic_entt().emplace<Name_component>(entity, "nanovdb_volume");
    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    auto primitives = create_primitives(entity);
    logic_update_proxy(entity, primitives);
    return entity;
}