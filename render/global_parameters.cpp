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
    update_directional_light();
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
    float minZ      = nearClip;
    float maxZ      = farClip; // 修正：直接等于 farClip

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    // Calculate split depths based on view camera frustum
    // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    // 1. 计算 Practical Split 深度
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        float p          = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
        float log        = minZ * std::pow(ratio, p);
        float uniform    = minZ + range * p;
        float d          = cascadeSplitLambda * (log - uniform) + uniform;
        cascadeSplits[i] = (d - nearClip) / clipRange;
    }
    // 目的是为零什么? 计算


    // 纯数学优化的紧密球心与半径计算（代替你原本的公式）
    // float k = std::sqrt(tanHalfFOVX * tanHalfFOVX + tanHalfFOVY * tanHalfFOVY); // 视锥体对角线斜率
    // float k2 = k * k;
    //
    // // 最完美的球心 Z 轴位置
    // float sphereCenterZ = 0.0f;
    // if (cFar * (1.0f - k2) > cNear) {
    //     sphereCenterZ = (cFar * (1.0f + k2) + cNear) / 2.0f;
    // } else {
    //     sphereCenterZ = cFar;
    // }
    //
    // // 最完美的紧密半径
    // float radius = std::sqrt((cFar - sphereCenterZ) * (cFar - sphereCenterZ) + cFar * cFar * k2);


    //
    // Calculate orthographic projection matrix for each cascade
    float lastSplitDist = 0.0;
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        float splitDist = cascadeSplits[i];

        // 2. 正确将 NDC 视锥体恢复到世界空间
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
            // 【修复 BUG 1】：修正 X, Y, Z 的赋值错误
            frustumCorners[j] = Eigen::Vector3f{
                invCorner.x() / invCorner.w(),
                invCorner.y() / invCorner.w(),
                invCorner.z() / invCorner.w()
            };
        }

        // 按当前级联截取视锥体片段
        for (uint32_t j = 0; j < 4; j++) {
            Eigen::Vector3f dist  = frustumCorners[j + 4] - frustumCorners[j];
            frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
            frustumCorners[j]     = frustumCorners[j] + (dist * lastSplitDist);
        }

        // 3. 计算【更稳定】的包围球中心和半径
        // 【修复 BUG 2】：直接使用几何计算得到的球心，防止旋转抖动
        Eigen::Vector3f frustumCenter = Eigen::Vector3f::Zero();
        for (uint32_t j = 0; j < 8; j++) {
            frustumCenter += frustumCorners[j];
        }
        frustumCenter /= 8.0f;

        // 计算包围球半径
        float radius = 0.0f;
        for (uint32_t j = 0; j < 8; j++) {
            float distance = (frustumCorners[j] - frustumCenter).norm();
            radius         = std::max(radius, distance);
        }
        // 稍微向上取整，增加一小圈边界缓冲区
        radius = std::ceil(radius * 16.0f) / 16.0f;
        // 还是没有做好级联, 能看到的 阴影 在远处消失的现象可以通过 扩大这里的半径来解决

        // 4. 处理你的核心需求：XY 轴使用半径，Z 轴使用自定义 Buffer
        Eigen::Vector3f lightDir = light.get_direction().normalized();

        // 你指定的自定义 Z 轴 Buffer
        float zNearBuffer = 150.0f; // 允许球心背后多远（Caster 范围） // 这里可以很有
        float zFarBuffer  = 50.0f;  // 允许球心前面延伸多远

        // 视点选择：强行让 View 矩阵的 LookAt 中心落在完美的球心上
        // 眼睛位置放在球心沿着光线反方向向后退 zNearBuffer 的地方
        Eigen::Vector3f lightPos        = frustumCenter - lightDir * zNearBuffer;
        Eigen::Matrix4f lightViewMatrix = eigenLookAt(lightPos,
                                                      frustumCenter,
                                                      Eigen::Vector3f(0.0f, 1.0f, 0.0f));

        // 在 Light View 空间中：
        // XY 轴的中心就是 (0,0)，边界由半径死死卡住
        float minX = -radius;
        float maxX = radius;
        float minY = -radius;
        float maxY = radius;

        // Z 轴总长：从眼睛位置（近平面 0.0）一直延伸到球心前方 zFarBuffer 的地方
        float totalZRange = zNearBuffer + zFarBuffer;

        // 调用你专为 DX/Vulkan 写的 ortho 投影函数
        // 此时近裁剪面设为 0.0f，远裁剪面设为总深度范围
        Eigen::Matrix4f lightOrthoMatrix = eigenOrthoDX_FlipY_StandardZ(minX, maxX,
                                                                        minY, maxY,
                                                                        0.0f, totalZRange);

        // 5. 存储并进行 Texel 对齐（防止平移抖动）
        // 为了做到极致的无抖动，建议在此处加上对齐逻辑（可选，若需要可参考下方提示）

        split_depth[i]          = (nearClip + splitDist * clipRange) * 1.0f;
        light_viewProjMatrix[i] = lightOrthoMatrix * lightViewMatrix;
        light_frustum_planes[i] = get_Frustum_Planes(light_viewProjMatrix[i]);
        lastSplitDist           = cascadeSplits[i];
    }
}
