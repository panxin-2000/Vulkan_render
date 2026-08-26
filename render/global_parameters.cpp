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

        Eigen::Vector3f lightDir = light.get_direction();


        // 1. 依然先计算光照视矩阵，但这次把相机放在视锥体中心 (或者稍微靠后一点的保底位置)
        // 我们假定一个基础的灯光位置，方向由 lightDir 决定
        Eigen::Vector3f baseLightPos    = frustumCenter - lightDir * 1.0f;
        Eigen::Matrix4f lightViewMatrix = eigenLookAt(baseLightPos, frustumCenter, Eigen::Vector3f(0.0f, 1.0f, 0.0f));

        // 2. 将视锥体的 8 个顶点全部转换到灯光空间 (Light Space)
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();

        for (uint32_t j = 0; j < 8; j++) {
            // 将顶点乘以 lightViewMatrix
            Eigen::Vector4f posLightSpace = lightViewMatrix * Eigen::Vector4f(frustumCorners[j].x(),
                                                                              frustumCorners[j].y(),
                                                                              frustumCorners[j].z(), 1.0f);

            // 寻找灯光空间下的最大最小值 (AABB)
            minX = std::min(minX, posLightSpace.x());
            maxX = std::max(maxX, posLightSpace.x());
            minY = std::min(minY, posLightSpace.y());
            maxY = std::max(maxY, posLightSpace.y());
            minZ = std::min(minZ, posLightSpace.z());
            maxZ = std::max(maxZ, posLightSpace.z());
        }

        // 3. 给 Z 轴（深度）加一个保底的缓冲区 (防止刚好在视锥体外面的大物体遮挡光线却被裁剪了)
        float zBuffer = 50.0f; // 根据你的场景规模调整
        minZ          -= zBuffer;

        float shadowMapResolution = 2048.0f;
        float worldTexelSizeX     = (maxX - minX) / shadowMapResolution;
        float worldTexelSizeY     = (maxY - minY) / shadowMapResolution;

        minX = std::floor(minX / worldTexelSizeX) * worldTexelSizeX;
        maxX = std::floor(maxX / worldTexelSizeX) * worldTexelSizeX;
        minY = std::floor(minY / worldTexelSizeY) * worldTexelSizeY;
        maxY = std::floor(maxY / worldTexelSizeY) * worldTexelSizeY;


        Eigen::Matrix4f lightOrthoMatrix = eigenOrthoDX_FlipY_StandardZ(
                                                                        minX, // left
                                                                        maxX, // right
                                                                        minY, // bottom
                                                                        maxY, // top
                                                                        0.0f,
                                                                        // near plane (offset to 0 as your function expects)
                                                                        maxZ - minZ
                                                                        // far plane (the total depth range of the bounding box)
                                                                       );

        // Store split distance and matrix in cascade
        split_depth[i]          = (nearClip + splitDist * clipRange) * -1.0f;
        light_viewProjMatrix[i] = lightOrthoMatrix * lightViewMatrix;

        lastSplitDist = cascadeSplits[i];
    }
}
