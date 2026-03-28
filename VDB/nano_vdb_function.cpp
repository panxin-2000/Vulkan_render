//
// Created by 潘鑫 on 2026/3/16.
//


#define NANOVDB_USE_OPENVDB
#include <openvdb/openvdb.h>

#define PNANOVDB_C
#define PNANOVDB_HDDA
#include "../render/shader/PNanoVDB.h"

#include <openvdb/tools/LevelSetSphere.h> // replace with your own dependencies for generating the OpenVDB grid
#include <fstream>
#include <nanovdb/tools/CreateNanoGrid.h> // converter from OpenVDB to NanoVDB (includes NanoVDB.h and GridManager.h)
#include <nanovdb/io/IO.h>

void convert(const std::string &filename) {
    try {
        // Create an OpenVDB grid of a sphere at the origin with radius 100 and voxel size 1.
        auto srcGrid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(100.0f, openvdb::Vec3f(0.0f), 1.0f);
        auto handle  = nanovdb::tools::createNanoGrid(*srcGrid);
        // Convert from OpenVDB to NanoVDB and return a shared pointer to a GridHandle.
        const auto *nanoGrid = handle.grid<float>(); // Get a (raw) pointer to the NanoVDB grid form the GridManager.
        if (!nanoGrid)
            throw std::runtime_error("GridHandle does not contain a grid with value type float");

        // A. 获取内存指针和大小 (这部分数据可以直接传给 C 函数)
        const auto rawBuffer = handle.data();
        uint64_t bufferSize  = handle.size();

        int count = handle.gridCount();


        // B. 构建 pnanovdb 专用的 Buffer 句柄
        pnanovdb_buf_t buf;
        buf.data = static_cast<uint32_t *>(rawBuffer); // PNanoVDB 内部按 32位对齐偏移

        pnanovdb_grid_handle_t grid = {0}; // 指向第一个网格
        auto tree                   = pnanovdb_grid_get_tree(buf, grid);
        // C. 获取 Grid 句柄 (通常第一个 Grid 偏移量为 0)
        auto root = pnanovdb_tree_get_root(buf, tree);
        pnanovdb_readaccessor_t acc;
        pnanovdb_readaccessor_init(&acc, root);

        pnanovdb_hdda_init()

        // 假设我们要查询一个整数坐标 (10, 20, 30)

        // 使用初始化好的 acc 进行快速读取

        const pnanovdb_vec3_t origin{0, 0, 0};
        const pnanovdb_vec3_t direction{0, 0, 1};
        // 1. 初始化射线 (在索引空间 index space)
        pnanovdb_ray_t ray;
        // pnanovdb_ray_init(&ray, &eye_idx, &dir_idx); // 传入索引空间的起点和方向
        // ray.t0 = near_dist;
        // ray.t1 = far_dist;
        //
        // // 2. 初始化 HDDA 状态机
        pnanovdb_hdda_t hdda;
        // // 参数：射线，以及你要步进的起始节点层级（通常是根节点层级）
        pnanovdb_hdda_init(&hdda, &origin, 0, &direction, 100, 1);


        // 2. 开始 HDDA 步进
        // HDDA 会跳过空白节点，直接带你跳到有数据的节点边界
        while (pnanovdb_hdda_step(&hdda)) {
            // 3. 在当前位置采样
            // 1. 获取体素在 Buffer 中的相对地址
            // 参数：grid_type, buffer_ptr, accessor_ptr, ijk_coords
            pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(
                PNANOVDB_GRID_TYPE_FLOAT, // 确保网格类型匹配
                buf,                      // pnanovdb_buf_t
                &acc,                     // 初始化好的访问器
                &hdda.ijk                      // pnanovdb_vec3_i32_t 坐标
            );

            // 2. 从该地址读取浮点值
            float value = pnanovdb_read_float(buf, address);

            // nanovdb_vec3_t iCurrP = nanovdb_vec3_addf(iPos, nanovdb_vec3_mulf(iDir, hdda.t));
            // nanovdb_coord_t ijk   = nanovdb_vec3_to_coord(iCurrP);
            // float value           = nanovdb_read_float(pnanovdb_buf_data, acc, ijk);

            // 4. 判定条件：只要值大于阈值，说明“击中”了
            // if (value > 0.001f) {
            // fragColor = vec4(1.0, 0.0, 0.0, 1.0); // 渲染成红色
            // return;                               // 立即停止，不再往后走
            // }
        }

        // Access and print out a single value (inside the level set) from both grids
        // printf("OpenVDB cpu: %4.2f\n", srcGrid->tree().getValue(openvdb::Coord(99, 0, 0)));
        // printf("NanoVDB cpu: %4.2f\n", dstGrid->tree().getValue(nanovdb::Coord(99, 0, 0)));


        // 5. 基础查询测试 (类似于之前 C 语言版本的采样)
        auto accessor = nanoGrid->getAccessor();

        nanovdb::Coord ijk(0, 0, 0);
        float val = accessor.getValue(ijk);


        // 4. 创建一个读取访问器 (Read Accessor)
        // 访问器会缓存层级节点，是加速查询的关键
        pnanovdb_readaccessor_t acc;
        pnanovdb_readaccessor_init(&acc, pnanovdb_grid_get_root(buf, handle));


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
