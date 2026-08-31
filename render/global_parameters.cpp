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


struct CascadeSplit {
    float nearPlane;
    float farPlane;
};


float calculateCascadeRadiusFromProj(const Eigen::Matrix4f &mainProjMatrix, float n, float f) {
    // 1. 从主相机投影矩阵中直接提取视锥体的缩放率
    // 注意：Eigen 的矩阵索引是 (row, col)。
    // 如果你的投影矩阵经过了特殊的 FlipY，或者有些管线在 P(1,1) 取了负号，请取绝对值 std::abs
    float k = 1.0f / std::abs(mainProjMatrix(1, 1)); // 对应垂直 tan(vfov / 2)
    float m = 1.0f / std::abs(mainProjMatrix(0, 0)); // 对应水平 tan(hfov / 2)

    // 2. 核心常数半径几何公式
    float range    = f - n;
    float fSquared = f * f;

    // 计算视锥体完美的最小外接球半径
    float radius = 0.5f * std::sqrt(range * range + 4.0f * (k * k + m * m) * fSquared);

    return radius;
}


std::vector<CascadeSplit> calculateSplits(float totalNear, float totalFar, int numCascades, float lambda = 0.95f) {
    std::vector<float> splitDistances(numCascades + 1);
    splitDistances[0]           = totalNear;
    splitDistances[numCascades] = totalFar;


    // 核心 PSSM / Practical Split Scheme 公式
    for (int i = 1; i < numCascades; ++i) {
        float f            = (i) / static_cast<float>(numCascades);
        float logSplit     = totalNear * std::pow(totalFar / totalNear, f);
        float uniformSplit = totalNear + (totalFar - totalNear) * f;
        splitDistances[i]  = lambda * logSplit + (1.0f - lambda) * uniformSplit;
    }

    // splitDistances[i]  = splitDistances[i - 1] + (lambda * logSplit + (1.0f - lambda) * uniformSplit) * (
    //                      totalFar - totalNear);

    // 转换为你的函数所需要的每一份的 [n, f]
    std::vector<CascadeSplit> result;
    for (int i = 0; i < numCascades; ++i) {
        CascadeSplit split;
        split.nearPlane = splitDistances[i];
        split.farPlane  = splitDistances[i + 1];
        result.push_back(split);
    }

    return result;
}


/**
 * @brief 计算视锥体（子级联）的世界空间完美最小包围球心
 * @param mainViewMatrix 主相机的 View 矩阵
 * @param mainProjMatrix 主相机的 Projection 矩阵（支持透视投影）
 * @param n 当前级联的近平面距离 (Near)
 * @param f 当前级联的远平面距离 (Far)
 * @return Eigen::Vector3f 最小包围球的世界空间中心坐标
 */
Eigen::Vector3f calculateCascadeSphereCenter(
    const Eigen::Matrix4f &mainViewMatrix,
    const Eigen::Matrix4f &mainProjMatrix,
    float n, float f) {
    // 1. 从主相机 View 矩阵中逆向提取相机的世界位置和朝向
    Eigen::Matrix4f invView = mainViewMatrix.inverse();

    // 逆矩阵的第四列前三个分量就是相机在世界空间的 Position
    Eigen::Vector3f mainCameraPos = invView.block<3, 1>(0, 3);

    // 逆矩阵的第三列（Z轴）取反，就是相机在世界空间的正前方向量 (Look Direction)
    // 注：如果是右手坐标系，相机看向 -Z，所以逆矩阵的第三列向量指向相机背后，我们需要取负号得到向前向量
    Eigen::Vector3f mainCameraLookDir = -invView.block<3, 1>(0, 2);
    mainCameraLookDir.normalize(); // 确保归一化

    float tanHalfVfov = 1.0f / std::abs(mainProjMatrix(1, 1));
    float tanHalfHfov = 1.0f / std::abs(mainProjMatrix(0, 0));

    // 几何推导中的关键系数：(tan(vfov/2)^2 + tan(hfov/2)^2)
    float gauss = tanHalfVfov * tanHalfVfov + tanHalfHfov * tanHalfHfov;

    float D     = 0.0f;
    float range = f - n;
    float sum   = f + n;

    // 条件判定：如果视锥体开角够大，外接球就是最小包围球
    if (gauss >= (range / sum)) {
        // 情况 A：标准外接球公式（通过视锥体全部 8 个顶点）
        D = 0.5f * sum * (1.0f + gauss);
    } else {
        // 情况 B：视锥体过于细长（通常发生在 FoV 很小，或者级联切分得极远时）
        // 此时强行包裹近平面会导致球体巨大。最优解是球心直接落在远平面中心，向后完美包裹整个视锥
        D = f;
    }

    // =================================================================
    // 4. 计算并返回世界坐标
    // =================================================================
    Eigen::Vector3f minimumBoundingCenter = mainCameraPos + mainCameraLookDir * D;

    return minimumBoundingCenter.array().floor(); // 部分方向上有用,部分方向上没有用
}


bool Global_parameters::update_directional_light() {
#define SHADOW_MAP_CASCADE_COUNT 4

    float nearClip = 0;
    float farClip  = 0;
    getPerspectiveClips(projection_matrix, nearClip, farClip);


    auto cascades = calculateSplits(nearClip, farClip, SHADOW_MAP_CASCADE_COUNT);
    // 纯数学优化的紧密球心与半径计算（代替你原本的公式）

    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        Eigen::Vector3f frustumCenter = calculateCascadeSphereCenter(view_matrix,
                                                                     projection_matrix,
                                                                     cascades[i].nearPlane,
                                                                     cascades[i].farPlane);

        // 计算包围球半径
        float radius = calculateCascadeRadiusFromProj(projection_matrix, cascades[i].nearPlane, cascades[i].farPlane);
        // 计算“常数级联半径”  能确保半径 不再 变化

        radius = radius * 1.25; // 不知道为什么需要放大一点点,好像有一点不同 ,
        // frustumCenter 的平滑移动是导致抖动的“罪魁祸首”（诱因），
        // 而 radius（半径）如果没有锁死，则是放大这种抖动的“帮凶”

        // 4. 处理你的核心需求：XY 轴使用半径，Z 轴使用自定义 Buffer
        Eigen::Vector3f lightDir = light.get_direction().normalized();

        // 你指定的自定义 Z 轴 Buffer
        float zNearBuffer = 150.0f; // 允许球心背后多远（Caster 范围） // 这里可以很有
        float zFarBuffer  = 50.0f;  // 允许球心前面延伸多远

        Eigen::Matrix4f lightViewMatrix = eigenLookAt(frustumCenter,
                                                      frustumCenter + lightDir,
                                                      Eigen::Vector3f(0.0f, 1.0f, 0.0f));


        // 调用你专为 DX/Vulkan 写的 ortho 投影函数
        // 此时近裁剪面设为 0.0f，远裁剪面设为总深度范围
        Eigen::Matrix4f lightOrthoMatrix = eigenOrthoDX_FlipY_StandardZ(-radius, +radius,
                                                                        -radius, +radius,
                                                                        -radius - zNearBuffer,
                                                                        +radius + zFarBuffer);

        split_depth[i]          = cascades[i].farPlane;
        light_viewProjMatrix[i] = lightOrthoMatrix * lightViewMatrix;
        light_frustum_planes[i] = get_Frustum_Planes(light_viewProjMatrix[i]);
    }
}
