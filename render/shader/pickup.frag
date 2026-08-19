/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"
#include "pbr_material.glsl"


layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inLightVec;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec4 inShadowCoord;
layout (location = 5) in vec3 inWorldPos;
layout (location = 6) flat in uint material_index;
layout (location = 7) flat in uint instance_index;
layout (location = 8) flat in uint entity;



layout (location = 0) out uint out_entity_R32_UINT;



void main()
{
    out_entity_R32_UINT = entity;
}
