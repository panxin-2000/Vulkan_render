#include "time_measure.h"
#include "gtest/gtest.h"

#define NANOVDB_USE_OPENVDB  // 这一句还是很关键的,否则 openToNanoVDB 总是提示找不到
#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/Composite.h>

#include <openvdb/tools/LevelSetSphere.h> // replace with your own dependencies for generating the OpenVDB grid
#include <fstream>
#include <nanovdb/io/IO.h>
#include <nanovdb/math/SampleFromVoxels.h>
#include <nanovdb/tools/CreateNanoGrid.h> // converter from OpenVDB to NanoVDB (includes NanoVDB.h and GridManager.h)


TEST(openvdb, create_Sphere) {
    openvdb::initialize();

    // Create a FloatGrid and populate it with a narrow-band
    // signed distance field of a sphere.
    openvdb::FloatGrid::Ptr grid =
            openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
                                                                     /*radius=*/50.0, /*center=*/
                                                                         openvdb::Vec3f(1.5, 2, 3),
                                                                         /*voxel size=*/0.5, /*width=*/4.0);

    // Associate some metadata with the grid.
    grid->insertMeta("radius", openvdb::FloatMetadata(50.0));

    // Name the grid "LevelSetSphere".
    grid->setName("LevelSetSphere");

    openvdb::FloatGrid::Ptr grid_b =
            openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
                                                                     /*radius=*/50.0, /*center=*/
                                                                         openvdb::Vec3f(1.5, 46, 3),
                                                                         /*voxel size=*/0.5, /*width=*/4.0);
    auto count   = grid->activeVoxelCount();
    auto count_b = grid_b->activeVoxelCount();
    auto bound   = grid->evalActiveVoxelDim(); // 获取包围盒
    // 2. 转换为物理世界包围盒 (openvdb::BBoxd)
    auto worldBox = grid->transform().indexToWorld(bound);

    struct CubicSmoothUnionOp {
        float k;
        float inv_k;
        float inv_6;

        explicit CubicSmoothUnionOp(float smoothness) : k(smoothness) {
            inv_k = 1.0f / k;
            inv_6 = 1.0f / 6.0f;
        }

        inline void operator()(const float &a, const float &b, float &result) const {
            float diff = std::abs(a - b);

            // 1. 如果两点距离超过了平滑半径，直接退化为普通硬布尔，不进行后续浮点乘法
            if (diff >= k) {
                result = std::min(a, b);
                return;
            }

            // 2. 核心修正版多项式计算（完全由基础乘减法构成，CPU 指令周期极短）
            float h  = 1.0f - diff * inv_k;
            float h3 = h * h * h; // 连乘效率远高于 std::pow

            result = std::min(a, b) - h3 * k * inv_6;
        }
    };

    openvdb::FloatGrid::Ptr grid_result = openvdb::FloatGrid::create();
    constexpr float voxelSize           = 0.02f;
    grid_result->setTransform(openvdb::math::Transform::createLinearTransform(voxelSize));

    grid_result->tree().combine2(grid->tree(), grid_b->tree(), CubicSmoothUnionOp(0.2));
    grid_result->tree().prune();

    // Create a VDB file object and write out the grid.
    {
        ScopedTimer timer("openvdb::tools::csgIntersection");
        openvdb::tools::compMul(*grid, *grid_b);
        // openvdb::tools::compMul(*gridA, *gridB);
        // csgDifference 是有顺序的，其他的操作是没有顺序的
    }


    openvdb::io::File("cmake-build-debug/mygrids.vdb").write({grid_result});
}


#include <iostream>
#include <librealsense2/rs.hpp>
#include <openvdb/openvdb.h>

TEST(openvdb, realsence) {
    // 1. 初始化 OpenVDB 环境
    openvdb::initialize();
    GTEST_SKIP();

    // 创建一个 FloatGrid 用来存储空间地图（此处以存储密度值/权重为例）
    openvdb::FloatGrid::Ptr vdbGrid = openvdb::FloatGrid::create(/*背景值=*/0.0f);

    vdbGrid->setGridClass(openvdb::GRID_FOG_VOLUME);

    // 关键配置：设置体素大小（Voxel Size）。例如 0.02 代表每个体素边长为 2 厘米 (0.02米)
    // 体素越小，分辨率越高，但 3ms 内能处理的点数就越少。2cm~5cm 是实时避障的黄金尺寸
    float voxelSize = 0.02f;
    vdbGrid->setTransform(openvdb::math::Transform::createLinearTransform(voxelSize));

    // 获取高速缓存访问器，这是在循环中 O(1) 级写入体素的绝对核心
    auto accessor = vdbGrid->getAccessor();

    // 2. 配置并启动 RealSense 深度流
    rs2::pipeline pipe;
    rs2::config cfg;
    // 推荐使用 640x480 @ 30FPS，数据量刚好适合高频实时更新
    cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
    pipe.start(cfg);

    // 定义 RealSense 的点云处理类
    rs2::pointcloud pc;
    rs2::points points;

    std::cout << "RealSense to OpenVDB pipeline running..." << std::endl;

    while (true) {
        // 等待捕获最新的一帧深度数据
        rs2::frameset frames   = pipe.wait_for_frames();
        rs2::depth_frame depth = frames.get_depth_frame();

        // 将 2D 深度图数据计算转化为 3D 点云数组 (包含 x, y, z 物理坐标，单位为米)
        points                      = pc.calculate(depth);
        const rs2::vertex *vertices = points.get_vertices();
        size_t totalPoints          = points.size();

        // 3. 高速流式注入 OpenVDB
        // 注：如果在追求极致性能的生产环境，此循环可以配合 tbb::parallel_for 并行化
        for (size_t i = 0; i < totalPoints; ++i) {
            // 必须剔除无效深度（RealSense 测距盲区或被遮挡的点 Z 轴会显示为 0）
            if (vertices[i].z <= 0.0f) continue;

            // 提取相机坐标系下的 3D 物理坐标 [单位：米]
            openvdb::Vec3R worldPos(vertices[i].x, vertices[i].y, vertices[i].z);

            // 【核心步骤】利用 Transform 将物理坐标秒转为 VDB 的三维整数网格坐标
            auto voxelCoord = vdbGrid->worldToIndex(worldPos);

            openvdb::Coord xyzCoord = openvdb::Coord::round(voxelCoord); // 这是需要 进行 四舍五入 因为想对齐格子的中心
            // 强行写入体素值。1.0f 代表当前空间有障碍物/被占据
            // 如果 RealSense 采集到多个点落入同一个 2cm 的方块内，它们会在这里重叠覆盖，实现天然的降噪
            accessor.setValue(xyzCoord, 1.0f);
        }

        // 打印当前 VDB 地图里一共激活（占据）了多少个体素
        std::cout << "[VDB Map] Active Voxel Count: " << vdbGrid->activeVoxelCount() << std::endl;
    }
}


void convert(const std::string &filename) {
    try {
        // Create an OpenVDB grid of a sphere at the origin with radius 100 and voxel size 1.
        auto baseGrid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(100.0f, openvdb::Vec3f(0.0f), 1.0f);
        auto grid     = openvdb::gridPtrCast<openvdb::Vec3fGrid>(baseGrid);
        auto handle   = nanovdb::tools::openToNanoVDB(grid);
        // 一个 nanovdb::GridHandle 可以包含多个网格 如密度场、温度场、速度场等

        auto ptr  = handle.data();
        auto size = handle.bufferSize();


        // test(ptr, size);


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
