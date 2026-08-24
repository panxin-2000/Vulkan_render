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

bool Global_parameters::update_directional_light(const Eigen::Vector3f &v3) {
#define SHADOW_MAP_CASCADE_COUNT 4

    float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
    //
    // float nearClip  = camera.getNearClip();
    // float farClip   = camera.getFarClip();
    // float clipRange = farClip - nearClip;
    //
    // float minZ = nearClip;
    // float maxZ = nearClip + clipRange;
    //
    // float range = maxZ - minZ;
    // float ratio = maxZ / minZ;
    //
    // // Calculate split depths based on view camera frustum
    // // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    // for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
    //     float p          = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
    //     float log        = minZ * std::pow(ratio, p);
    //     float uniform    = minZ + range * p;
    //     float d          = cascadeSplitLambda * (log - uniform) + uniform;
    //     cascadeSplits[i] = (d - nearClip) / clipRange;
    // }
    //
    // // Calculate orthographic projection matrix for each cascade
    // float lastSplitDist = 0.0;
    // for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
    //     float splitDist = cascadeSplits[i];
    //
    //     glm::vec3 frustumCorners[8] = {
    //         glm::vec3(-1.0f, 1.0f, 0.0f),
    //         glm::vec3(1.0f, 1.0f, 0.0f),
    //         glm::vec3(1.0f, -1.0f, 0.0f),
    //         glm::vec3(-1.0f, -1.0f, 0.0f),
    //         glm::vec3(-1.0f, 1.0f, 1.0f),
    //         glm::vec3(1.0f, 1.0f, 1.0f),
    //         glm::vec3(1.0f, -1.0f, 1.0f),
    //         glm::vec3(-1.0f, -1.0f, 1.0f),
    //     };
    //
    //     // Project frustum corners into world space
    //     glm::mat4 invCam = glm::inverse(camera.matrices.perspective * camera.matrices.view);
    //     for (uint32_t j = 0; j < 8; j++) {
    //         glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[j], 1.0f);
    //         frustumCorners[j]   = invCorner / invCorner.w;
    //     }
    //
    //     for (uint32_t j = 0; j < 4; j++) {
    //         glm::vec3 dist        = frustumCorners[j + 4] - frustumCorners[j];
    //         frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
    //         frustumCorners[j]     = frustumCorners[j] + (dist * lastSplitDist);
    //     }
    //
    //     // Get frustum center
    //     glm::vec3 frustumCenter = glm::vec3(0.0f);
    //     for (uint32_t j = 0; j < 8; j++) {
    //         frustumCenter += frustumCorners[j];
    //     }
    //     frustumCenter /= 8.0f;
    //
    //     float radius = 0.0f;
    //     for (uint32_t j = 0; j < 8; j++) {
    //         float distance = glm::length(frustumCorners[j] - frustumCenter);
    //         radius         = glm::max(radius, distance);
    //     }
    //     radius = std::ceil(radius * 16.0f) / 16.0f;
    //
    //     glm::vec3 maxExtents = glm::vec3(radius);
    //     glm::vec3 minExtents = -maxExtents;
    //
    //     glm::vec3 lightDir        = normalize(-lightPos);
    //     glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter,
    //                                             glm::vec3(0.0f, 1.0f, 0.0f));
    //     glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f,
    //                                             maxExtents.z - minExtents.z);
    //
    //     // Store split distance and matrix in cascade
    //     cascades[i].splitDepth     = (camera.getNearClip() + splitDist * clipRange) * -1.0f;
    //     cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;
    //
    //     lastSplitDist = cascadeSplits[i];
    // }
}
