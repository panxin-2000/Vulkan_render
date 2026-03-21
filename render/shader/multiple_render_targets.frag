#version 450


#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (set = 2, binding = 1) uniform sampler2D samplerColor;
//layout (binding = 2) uniform sampler2D samplerNormalMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inWorldPos;


layout (location = 0) out vec4 outPosition_R16G16B16A16_SFLOAT;
layout (location = 1) out vec4 outNormal_R16G16B16A16_SFLOAT;
layout (location = 2) out vec4 outBaseColor_R8G8B8A8_UNORM;

float hash(int xy) {
    uint x = uint(xy);
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = (x >> 16u) ^ x;
    return float(x) / 4294967295.0;
}

// Multiple Render Targets
void main()
{
    outPosition_R16G16B16A16_SFLOAT = vec4(inWorldPos, 1.0);
    //outBaseColor = vec4(hash(gl_PrimitiveID + 1), hash(gl_PrimitiveID + 2), hash(gl_PrimitiveID + 3), 1.0);


    // Calculate normal in tangent space
    vec3 N = normalize(inNormal);
    //    vec3 T = normalize(inTangent);
    //    vec3 B = cross(N, T);
    //    mat3 TBN = mat3(T, B, N);
    //    vec3 tnorm = TBN * normalize(texture(samplerNormalMap, inUV).xyz * 2.0 - vec3(1.0));

    outNormal_R16G16B16A16_SFLOAT = vec4(N.xyz, 1.0);
    outBaseColor_R8G8B8A8_UNORM = texture(samplerColor, inUV);
}