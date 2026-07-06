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

#include "shader_component.h"
#include "update_push_constants_data.h"


void test(void *ptr, uint64_t size);

void add_nanovdb_to_gpu(entt::entity entity, void *ptr, uint64_t size) {
#define ALIGN_1024(size) (((size) + 1023) & ~1023)
    auto temp_ptr          = create_SSBO_buffer(ALIGN_1024(size));
    auto mem_copy_function = [ptr,size](void *dst) {
        memcpy(dst, ptr, size);
    };
    copy_mem_from_cpu_to_gpu(temp_ptr, mem_copy_function);

    set_render_parameter(entity, "nanovdb_buffer", temp_ptr);
}

void add_nanovdb_to_gpu(entt::entity entity) {
    auto srcGrid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(100.0f, openvdb::Vec3f(0.0f), 1.0f);
    nanovdb::GridHandle handle = nanovdb::tools::createNanoGrid(*srcGrid);
    auto ptr = handle.data();
    auto size = handle.bufferSize();
    add_nanovdb_to_gpu(entity, ptr, size);
    set_render_parameter(entity, "nanovdb_size", size);
}

void add_nanovdb_to_gpu(const entt::entity entity, const std::string &file_name) {
    std::filesystem::path filePath = file_name;
    std::string ext                = filePath.extension().string();
    if (ext == ".vdb") {
        openvdb::initialize();
        openvdb::io::File file(file_name);
        file.open();

        // 1. 存储所有从 OpenVDB 读取的网格指针
        openvdb::GridPtrVec vdbGrids;
        for (auto iter = file.beginName(); iter != file.endName(); ++iter) {
            vdbGrids.push_back(file.readGrid(iter.gridName()));
            std::cout << "Reading grid: " << iter.gridName() << std::endl;
        }
        file.close();

        // 2. 将所有网格批量转换为 NanoVDB GridHandles
        // NanoVDB 的 openToNanoVDB 支持传入 GridPtrVec
        std::vector<nanovdb::GridHandle<nanovdb::HostBuffer> > handles;


        // 遍历所有 Grid 名称
        for (auto iter = file.beginName(); iter != file.endName(); ++iter) {
            auto vdbGrid = file.readGrid(iter.gridName());

            if (vdbGrid) {
                std::cout << "Converting grid: " << iter.gridName() << std::endl;
                // 使用新版 createNanoGrid 函数

                auto baseGrid = file.readGrid(iter.gridName());

                if (baseGrid->isType<openvdb::FloatGrid>()) {
                    auto grid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);
                    handles.push_back(nanovdb::tools::createNanoGrid(*grid));
                } else if (baseGrid->isType<openvdb::Vec3fGrid>()) {
                    auto grid = openvdb::gridPtrCast<openvdb::Vec3fGrid>(baseGrid);
                    handles.push_back(nanovdb::tools::createNanoGrid(*grid));
                }
            }
        }
        file.close();
        // 将所有 Handle 一次性写入同一个 .nvdb 文件
        if (!handles.empty()) {
            // 1. 创建内存输出流
            std::stringstream ms;

            // 2. 将所有 handles 写入流（这会在内存中生成完整的 .nvdb 文件格式）
            nanovdb::io::writeGrids(ms, handles);

            // 3. 获取内存中的连续数据
            std::string data = ms.str();
            void *rawData    = data.data();
            size_t totalSize = data.size();
            add_nanovdb_to_gpu(entity, rawData, totalSize);
        }
    } else if (ext == ".nvdb") {
        auto handle = nanovdb::io::readGrid(file_name);
        if (handle.empty()) {
            const auto ptr  = handle.data();
            const auto size = handle.bufferSize();
            add_nanovdb_to_gpu(entity, ptr, size);
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
