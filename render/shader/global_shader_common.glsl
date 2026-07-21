//
// Created by 潘鑫 on 2026/3/13.
//
// 更新最缓慢
layout (set = 0, binding = 0) uniform sampler2D bindless_samplerColorMap[];

layout (set = 1, binding = 0) uniform global_parameters
{
    mat4 view;
    mat4 projection;
    mat4 invView;
    mat4 invProjection;
    mat4 inv_VP;
    vec3 viewPos;
    vec3 lightPos;
    vec4 screen_size;
};


vec2 octEncode(vec3 n) {
    // 1. L1 归一化：确保 |x| + |y| + |z| = 1
    float l1norm = abs(n.x) + abs(n.y) + abs(n.z);
    vec2 res = n.xy / l1norm;
    // normalize 是为了把向量投影到“球体”上，而 L1 归一化是为了把向量投影到“八面体”上。
    // 坐标绝对值之和等于 1 ，能被平整地展开成 2D 正方形的物理基础

    // 2. 处理下半球 (z < 0) 的翻折
    // 使用三元表达式或 sign 模拟。在现代编译器中，这种写法通常会被优化为 CMOV 或 BFI 指令
    vec2 signNotZero = vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);
    // 只是为了确定在那个象限，下层的时候 (1.0 - abs(res.yx)) 计算后得到的都是 正数 ，需要修改象限
    return (n.z >= 0.0) ? res : (1.0 - abs(res.yx)) * signNotZero;
    // 折叠后的边界必须与折叠前的边界重合  (1.0 - abs(res.yx)) // 其实重点是上下两层 边界靠近，插值时不会出问题

    // 展开的过程是什么呢？ 想象一个正方形，四个中点组成的正方形 组成了正八面体的 上半部分
    // 剩余的四个三角形组成了正八面体的下半部分
    // 四个中点组成的正方形 其 x+y 永远小于1
    // 剩余的四个三角形，其 |x| + |y| 用于大于1
    // 举一个例子，x = 0.5 时，y = 0.5  z = 0
    // 举一个例子，x = 0.25 时，y = 0.25  z = -0.5
    // 那么需要放置的位置时  x = 0.75 时，y = 0.75
    // 举一个例子，x = -0.25 时，y = 0.25  z = -0.5
    // 那么需要放置的位置时  x = -0.75 时，y = 0.75

    // 还需要之后写一个 C 语言版本的，最好能使用 矢量加速器

}

vec3 octDecode(vec2 v) {
    // 1. 根据 x, y 推导初步的 z
    vec3 n = vec3(v, 1.0 - abs(v.x) - abs(v.y));

    // 2. 如果 z < 0，利用原 xy 的正负号进行翻折还原
    // 这行逻辑实现了：当 n.z < 0 时，将 xy 翻折回球面
    vec2 s = vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);
    n.xy += (n.z < 0.0) ? (abs(n.yx) - 1.0) * s : vec2(0.0);

    return normalize(n);
}


mat4 calculate_matrix(vec3 instancePos, vec3 instanceDir) {
    vec3 forward = normalize(instanceDir);

    // 2. 定义世界坐标系的临时“上”方向
    vec3 worldUp = vec3(0.0, 1.0, 0.0);
    // 防止物体正向上导致叉乘为 0，做一个微小的兜底
    if (abs(dot(forward, worldUp)) > 0.99) {
        worldUp = vec3(0.0, 0.0, 1.0);
    }

    // 3. 叉乘构建互相正交的 X 轴 (right) 和 Y 轴 (up)
    vec3 right = normalize(cross(worldUp, forward));
    vec3 up = cross(forward, right);

    // 4. 在 GLSL 中实时动态构建 4x4 变换矩阵
    // 注意：GLSL 的 mat4 是 列主序 (Column-Major)，传参按列排列
    mat4 model = mat4(
    vec4(right, 0.0), // 第一列：X 轴 (旋转)
    vec4(up, 0.0), // 第二列：Y 轴 (旋转)
    vec4(forward, 0.0), // 第三列：Z 轴 (旋转)
    vec4(instancePos, 1.0) // 第四列：位移 (Position)
    );
    return model;
}

// 用正八面体做环境贴图
// vec3 R = reflect(-V, N);
// vec2 uv = octEncode(R) * 0.5 + 0.5; // 映射到 [0, 1] 范围
// // lod 对应粗糙度级别
// vec3 envColor = textureLod(u_OctahedralEnvMap, uv, lod).rgb;

// 关键挑战：处理边界接缝 (Seams)
// 在预过滤环境贴图时，八面体的边界是最大的挑战。
// 问题：硬件的线性过滤（Linear Filtering）在八面体 UV 边界处会采样到错误的像素（因为 UV 空间在这些地方是断开的）。
// 解决方案：
// Padding（填充）：在生成 2D 八面体贴图时，在每个边缘外扩展 1-2 个像素，并根据翻折逻辑将对应的颜色填进去。
// 坐标修正：在 Shader 采样前，对 UV 进行极其微小的缩放，使其避开最外层的像素边缘。
