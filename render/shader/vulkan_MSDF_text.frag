#version 450
#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (location = 0) in vec2 in_UV;


layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;

layout (set = 2, binding = 0) uniform sampler2D msdf;


float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}


vec2 sqr(vec2 x) { return x * x; } // squares vector components

float screenPxRange(vec2 in_UV) {
    float pxRange = 0.125; // set to distance field's pixel range
    vec2 unitRange = vec2(pxRange) / vec2(textureSize(msdf, 0));
    // If inversesqrt is not available, use vec2(1.0)/sqrt
    vec2 screenTexSize = inversesqrt(sqr(dFdx(in_UV)) + sqr(dFdy(in_UV)));
    //    inversesqrt 应该是可以用length 这个函数替代的
    // Can also be approximated as screenTexSize = vec2(1.0)/fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

void main()
{


    vec3 msd = texture(msdf, in_UV).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange(in_UV) * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    //    float opacity = smoothstep(screenPxDistance + 0.5, 0.0, 1.0);
    //  smoothstep 三次曲线，根据平滑
    //  smoothstep 是为了让文字好看（没锯齿）
    vec3 bgColor = vec3(0, 0, 0);
    vec3 fgColor = vec3(1, 1, 1);
    vec3 color = mix(bgColor, fgColor, opacity);
    outFragColor_B8G8R8A8_SRGB = vec4(color.rgb, 1.0);

}