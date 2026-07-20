#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/Composite.h>

#include "time_measure.h"
#include "gtest/gtest.h"
#include "quill/backend/StringFromTime.h"

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


    openvdb::io::File("mygrids.vdb").write({grid_result});
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
