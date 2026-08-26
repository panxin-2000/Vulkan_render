//
// Created by 潘鑫 on 2026/8/24.
//

#include "global_parameters.h"

bool Global_parameters::set_projection_matrix(const Eigen::Matrix4f &matrix) {
    projection_matrix = matrix;
    return true;
}

bool Global_parameters::set_inv_projection_matrix(const Eigen::Matrix4f &matrix) {
    inv_projection_matrix = matrix;
    return true;
}

bool Global_parameters::set_view_matrix(const Eigen::Matrix4f &matrix) {
    view_matrix = matrix;
    return true;
}

bool Global_parameters::set_inv_view_matrix(const Eigen::Matrix4f &matrix) {
    inv_view_matrix = matrix;
    return true;
}

bool Global_parameters::set_invVP(const Eigen::Matrix4f &matrix) {
    invVP = matrix;
    return true;
}

bool Global_parameters::set_world_camera_pos(const Eigen::Vector3f &v3) {
    world_camera_pos = {v3.x(), v3.y(), v3.z(), 0};
    return true;
}

bool Global_parameters::set_sun_light(const Eigen::Vector3f &v3) {
    light.set_color(1.0f, 0.98f, 0.95f);
    light.set_intensity(5.0f);
    auto tem = v3;
    tem.normalize();
    light.set_rotate({tem.x(), tem.y(), tem.z(), 0.0f});
    return true;
}

bool Global_parameters::set_screen_size(const Eigen::Vector2f &screen_size_t) {
    screen_size = {screen_size_t.x(), screen_size_t.y(), 0, 0};;
    return true;
}

void getPerspectiveClips(const Eigen::Matrix4f &proj, float &nearClip, float &farClip) {
    float A = proj(2, 2);
    float B = proj(2, 3);

    nearClip = B / A;
    farClip  = B / (A + 1.0f);
}

// 提取正交投影矩阵的 Near 和 Far (Z 范围)
void getOrthographicClips(const Eigen::Matrix4f &proj, float &nearClip, float &farClip) {
    float A = proj(2, 2);
    float B = proj(2, 3);

    nearClip = B / A;
    farClip  = (B + 1.0f) / A;
}


// --- 辅助函数：生成 LookAt 矩阵 (列主序) ---
Eigen::Matrix4f eigenLookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, const Eigen::Vector3f &up) {
    Eigen::Vector3f f = (center - eye).normalized();
    Eigen::Vector3f s = f.cross(up).normalized();
    Eigen::Vector3f u = s.cross(f);

    Eigen::Matrix4f mat = Eigen::Matrix4f::Identity();
    mat(0, 0)           = s.x();
    mat(0, 1)           = s.y();
    mat(0, 2)           = s.z();
    mat(1, 0)           = u.x();
    mat(1, 1)           = u.y();
    mat(1, 2)           = u.z();
    mat(2, 0)           = -f.x();
    mat(2, 1)           = -f.y();
    mat(2, 2)           = -f.z();

    mat(0, 3) = -s.dot(eye);
    mat(1, 3) = -u.dot(eye);
    mat(2, 3) = f.dot(eye);
    return mat;
}

// --- 辅助函数：生成正交投影矩阵 (对应 Vulkan/DirectX 深度范围) ---
Eigen::Matrix4f
eigenOrthoDX_FlipY_StandardZ(float left, float right, float bottom, float top, float zNear, float zFar) {
    Eigen::Matrix4f mat = Eigen::Matrix4f::Identity();

    // X 轴
    mat(0, 0) = 2.0f / (right - left);
    mat(0, 3) = -(right + left) / (right - left);

    // Y 轴：翻转 Y 轴
    mat(1, 1) = -2.0f / (top - bottom);
    mat(1, 3) = -(top + bottom) / (top - bottom);

    // Z 轴：标准 Vulkan/DX 深度映射 (zNear -> 0, zFar -> 1)
    mat(2, 2) = 1.0f / (zNear - zFar);
    mat(2, 3) = zNear / (zNear - zFar);

    // W 轴
    mat(3, 3) = 1.0f;

    return mat;
}


bool Global_parameters::update_directional_light() {
#define SHADOW_MAP_CASCADE_COUNT 4

    float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

    float cascadeSplitLambda = 0.95f;
    float nearClip           = 0;
    float farClip            = 0;
    getPerspectiveClips(projection_matrix, nearClip, farClip);

    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    // Calculate split depths based on view camera frustum
    // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        float p          = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
        float log        = minZ * std::pow(ratio, p);
        float uniform    = minZ + range * p;
        float d          = cascadeSplitLambda * (log - uniform) + uniform;
        cascadeSplits[i] = (d - nearClip) / clipRange;
    }
    //
    // Calculate orthographic projection matrix for each cascade
    float lastSplitDist = 0.0;
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        float splitDist = cascadeSplits[i];

        Eigen::Vector3f frustumCorners[8] = {
            Eigen::Vector3f(-1.0f, 1.0f, 0.0f),
            Eigen::Vector3f(1.0f, 1.0f, 0.0f),
            Eigen::Vector3f(1.0f, -1.0f, 0.0f),
            Eigen::Vector3f(-1.0f, -1.0f, 0.0f),
            Eigen::Vector3f(-1.0f, 1.0f, 1.0f),
            Eigen::Vector3f(1.0f, 1.0f, 1.0f),
            Eigen::Vector3f(1.0f, -1.0f, 1.0f),
            Eigen::Vector3f(-1.0f, -1.0f, 1.0f),
        };

        // Project frustum corners into world space
        // glm::mat4 invCam = glm::inverse(camera.matrices.perspective * camera.matrices.view);
        for (uint32_t j = 0; j < 8; j++) {
            Eigen::Vector4f invCorner = invVP * Eigen::Vector4f(frustumCorners[j].x(),
                                                                frustumCorners[j].y(),
                                                                frustumCorners[j].z(),
                                                                1.0f);
            frustumCorners[j] = Eigen::Vector3f{
                invCorner.x() / invCorner.w(), invCorner.x() / invCorner.w(), invCorner.x() / invCorner.w(),
            };
        }

        for (uint32_t j = 0; j < 4; j++) {
            Eigen::Vector3f dist  = frustumCorners[j + 4] - frustumCorners[j];
            frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
            frustumCorners[j]     = frustumCorners[j] + (dist * lastSplitDist);
        }

        Eigen::Vector3f frustumCenter = Eigen::Vector3f::Zero();
        for (uint32_t j = 0; j < 8; j++) {
            frustumCenter += frustumCorners[j];
        }
        frustumCenter /= 8.0f;


        // 计算包围球半径
        float radius = 0.0f;
        for (uint32_t j = 0; j < 8; j++) {
            // glm::length 替换为 Eigen 的 .norm()
            float distance = (frustumCorners[j] - frustumCenter).norm();
            radius         = std::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;


        Eigen::Vector3f lightDir = light.get_direction();


        // 1. 灯光视矩阵：初次建立，将相机放在视锥体中心 frustumCenter，朝向光线方向
        // 这步的唯一目的：确定方向，让灯光空间的 Z 轴与光线方向（lightDir）完美对齐！
        Eigen::Vector3f lightViewPos    = frustumCenter;
        Eigen::Matrix4f lightViewMatrix = eigenLookAt(lightViewPos, frustumCenter + lightDir,
                                                      Eigen::Vector3f(0.0f, 1.0f, 0.0f));

        // 2. 将视锥体的 8 个顶点转换到这个基础灯光空间中，寻找最紧凑的边界
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();

        for (uint32_t j = 0; j < 8; j++) {
            Eigen::Vector4f posLightSpace = lightViewMatrix * Eigen::Vector4f(frustumCorners[j].x(),
                                                                              frustumCorners[j].y(),
                                                                              frustumCorners[j].z(), 1.0f);
            minX = std::min(minX, posLightSpace.x());
            maxX = std::max(maxX, posLightSpace.x());
            minY = std::min(minY, posLightSpace.y());
            maxY = std::max(maxY, posLightSpace.y());
            minZ = std::min(minZ, posLightSpace.z()); // 视锥体距离光源最近的点
            maxZ = std::max(maxZ, posLightSpace.z()); // 视锥体距离光源最远的点
        }


        // 1. 确定你的 Vulkan 阴影贴图分辨率 (例如 2048x2048)
        float shadowMapResolution = 2048.0f;

        float fixedSize = 2.0f * radius;

        // 重新计算基于圆心对齐的紧凑中心
        float centerX = (minX + maxX) * 0.5f;
        float centerY = (minY + maxY) * 0.5f;

        // 建立恒定大小的临时边界
        minX = centerX - radius;
        maxX = centerX + radius;
        minY = centerY - radius;
        maxY = centerY + radius;

        // 然后运行上面的 Texel Snapping (单像素大小现在变成了固定的 fixedSize / shadowMapResolution)
        float texelSize = fixedSize / shadowMapResolution;
        minX            = std::floor(minX / texelSize) * texelSize;
        minY            = std::floor(minY / texelSize) * texelSize;
        maxX            = minX + fixedSize;
        maxY            = minY + fixedSize;


        // 3. 🎯 核心改变：自定义 Z 轴的“远程遮挡物捕捉范围”
        // 此时以 frustumCenter 为原点，光线背后（即玩家身后）的物体在灯光空间中是负 Z 方向。
        // 我们可以把 Near 缓冲区调得很大，从而把身后非常遥远的巨大建筑也装进阴影相机里！
        float zNearBuffer = 150.0f; // 允许光线背后多远（例如150米）的物体也能产生阴影投射进来
        float zFarBuffer  = 50.0f;  // 允许视锥体后面延伸多远

        // 计算绝对的灯光空间 Z 轴边界
        float orthographicNearZ = minZ - zNearBuffer;
        float orthographicFarZ  = maxZ + zFarBuffer;

        // 计算总的深度范围（也就是你的正交包围盒在光线方向上的总长度）
        float totalDepthRange = orthographicFarZ - orthographicNearZ;

        // 4. 构建你的专属 DirectX/Vulkan 正交投影矩阵
        // 因为你的函数格式固定了近平面传入 0.0f，远平面传入总跨度，所以我们直接传 totalDepthRange
        Eigen::Matrix4f lightOrthoMatrix = eigenOrthoDX_FlipY_StandardZ(
                                                                        minX,           // Left
                                                                        maxX,           // Right
                                                                        minY,           // Bottom
                                                                        maxY,           // Top
                                                                        0.0f,           // 函数期望的相对近平面 0
                                                                        totalDepthRange // 函数期望的相对远平面（总深度）
                                                                       );
        // 🔥 关键的一步补偿：因为我们在正交矩阵里把近平面 offset 到了 0，
        // 我们必须在物理空间中，把灯光相机的位置（lightViewMatrix）沿着光线反方向往后退 orthographicNearZ 的距离！
        // 这样才能保证现实世界中的物体在投影时，坐标能和正交矩阵的 [0, totalDepthRange] 严格对齐。
        // 1. 明确定义你想要相机往后退多少距离（绝对值）
        // 此时以 frustumCenter 为基准，minZ 是负数，所以 -minZ 就是它到中心的绝对距离
        float backDistance = std::abs(minZ) + zNearBuffer;

        // 2. 🎯 正确且直观的坐标计算：
        // 要让相机后退，必须【减去】光线方向（逆着光线走）
        Eigen::Vector3f adjustedLightPos = frustumCenter - lightDir * backDistance;
        // 3. 重新建立最终的视矩阵
        // 相机位置在 adjustedLightPos，朝向依然顺着光线方向
        lightViewMatrix = eigenLookAt(adjustedLightPos, adjustedLightPos + lightDir, Eigen::Vector3f(0.0f, 1.0f, 0.0f));

        // Store split distance and matrix in cascade
        split_depth[i]          = (nearClip + splitDist * clipRange) * -1.0f;
        light_viewProjMatrix[i] = lightOrthoMatrix * lightViewMatrix;
        light_frustum_planes[i] = get_Frustum_Planes(light_viewProjMatrix[i]);
        lastSplitDist           = cascadeSplits[i];
    }
}
