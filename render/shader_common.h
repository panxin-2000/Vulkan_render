//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_SHADER_COMMON_H
#define HELLO_MAC_SHADER_COMMON_H
#define GLEW_STATIC
#include <GL/glew.h>

#include <base_element/point_3.h>

#define NULL_GPU_INDEX 0

// 渲染层级（控制绘制顺序，如UI > 角色 > 场景）
enum class RenderLayer {
    Background, // 背景
    World,      // 场景物体
    Character,  // 角色
    Weapon,     // 武器
    UI          // 界面
};

// 混合模式（透明/不透明渲染）
enum class BlendMode {
    Opaque,     // 不透明（默认）
    AlphaBlend, // 阿尔法混合（普通透明）
    Additive,   // 加法混合（发光效果）
    Multiply    // 乘法混合（暗化效果）
};

// 阴影模式
enum class ShadowMode {
    CastAndReceive, // 投射并接收阴影（默认）
    CastOnly,       // 仅投射阴影
    ReceiveOnly,    // 仅接收阴影
    None            // 无阴影
};

enum GPUPrimType : int8_t {
    GPU_PRIM_POINTS,
    GPU_PRIM_LINES,
    GPU_PRIM_TRIS,
    GPU_PRIM_LINE_STRIP,
    GPU_PRIM_LINE_LOOP, /* GL has this, Vulkan and Metal do not */
    GPU_PRIM_TRI_STRIP,
    GPU_PRIM_TRI_FAN, /* Metal API does not support this. */

    /* Metal API does not support ADJ primitive types but
     * handled via the geometry-shader-alternative path. */
    GPU_PRIM_LINES_ADJ,
    GPU_PRIM_TRIS_ADJ,
    GPU_PRIM_LINE_STRIP_ADJ,

    GPU_PRIM_NONE,
};


struct shader_and_share {
    unsigned int shader;
    uint16_t shared_number;
};


struct buffer_and_share {
    unsigned int buffer;
    uint16_t shared_number;
};

class logic_render_data;

using Vertices_type = std::shared_ptr<std::vector<Point_3> >;
using Indices_type = std::shared_ptr<std::vector<unsigned int> >;

enum Uniforms_type {
    gl_bool,
    gl_int,
    gl_float,
    gl_vec2,
    gl_vec3,
    gl_vec4,
    gl_mat2,
    gl_mat3,
    gl_mat4,
    gl_byte,
    gl_short,
    gl_int_vec2,
    gl_int_vec3,
    gl_int_vec4,
    gl_unint,
    gl_unint_vec2,
    gl_unint_vec3,
    gl_unint_vec4,
};


union data_value_or_ptr {
    bool bool_val;
    int int_val;
    float float_val;
    float vec_2[2];
    float vec_3[3];
    float vec_4[4];
    float mat_2[4];
    float mat_3[9];
    float mat_4[16];
};

int get_glenum_length(GLenum type);

struct VertexAttrib {
    GLint size;
    GLenum type;
    GLboolean normalized;
    /**
     *
     * @param size 表示有几个数据
     * @param type 类型，表示其中单个数据的类型
     * @param normalized 是否需要归一化
     * @param stride 间隔，重新下一个数据需要间隔多远
     * @param pointer 访问时是否需要偏移
     */
    VertexAttrib(GLint size,
                 GLenum type,
                 GLboolean normalized
    ) : size(size), type(type), normalized(normalized) {
    }
};


#endif //HELLO_MAC_SHADER_COMMON_H
