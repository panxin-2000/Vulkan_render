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

    float clipRange = farClip - nearClip; // 这里需要减小 , 但是呢?

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
    // 目的是为零什么? 计算
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
                invCorner.x() / invCorner.w(), invCorner.y() / invCorner.w(), invCorner.z() / invCorner.w(),
            };
        }

        for (uint32_t j = 0; j < 4; j++) {
            Eigen::Vector3f dist  = frustumCorners[j + 4] - frustumCorners[j];
            frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
            frustumCorners[j]     = frustumCorners[j] + (dist * lastSplitDist);
        }
        //
        // Get frustum center
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

        Eigen::Vector3f maxExtents = Eigen::Vector3f::Constant(radius);
        Eigen::Vector3f minExtents = -maxExtents;

        // 假设 lightPos 是 Eigen::Vector3f 类型的灯光方向或位置
        Eigen::Vector3f lightDir = light.get_direction();

        float zNearBuffer = 150.0f; // 允许光线背后多远（例如150米）的物体也能产生阴影投射进来
        float zFarBuffer  = 50.0f;  // 允许视锥体后面延伸多远


        Eigen::Matrix4f lightViewMatrix = eigenLookAt(frustumCenter - lightDir * maxExtents.z(),
                                                      frustumCenter,
                                                      Eigen::Vector3f(0.0f, 1.0f, 0.0f));

        // 替换 glm::ortho (使用上面专门为 DX/Vulkan 写的函数)
        Eigen::Matrix4f lightOrthoMatrix = eigenOrthoDX_FlipY_StandardZ(minExtents.x(), maxExtents.x(),
                                                                        minExtents.y(), maxExtents.y(),
                                                                        0.0f, maxExtents.z() - minExtents.z());

        // Store split distance and matrix in cascade
        split_depth[i]          = (nearClip + splitDist * clipRange) * -1.0f;
        light_viewProjMatrix[i] = lightOrthoMatrix * lightViewMatrix;
        light_frustum_planes[i] = get_Frustum_Planes(light_viewProjMatrix[i]);
        lastSplitDist           = cascadeSplits[i];
    }
}
