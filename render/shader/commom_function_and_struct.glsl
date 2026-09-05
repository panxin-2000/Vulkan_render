#ifndef COMMOM_FUNCTION_AND_STRUCT_INCLUDED
#define COMMOM_FUNCTION_AND_STRUCT_INCLUDED

struct VkDrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
};
const float PI = 3.14159265359;

#define EIGEN_INDEX(m, row, col) (m)[col][row]


struct Frustum {
    vec4 frustum_planes[6];
};

#ifdef TARGET_MOBILE
#define PREVENT_DIV0(n, d, magic)   ((n) / max(d, magic))
#else
#define PREVENT_DIV0(n, d, magic)   ((n) / (d))
#endif

// ACES 色调映射
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Filmic 胶片感色调映射
vec3 FilmicHejlTonemap(vec3 x) {
    vec3 max_x = max(vec3(0.0), x - vec3(0.004)); // 防止暗部溢出
    // 该公式内部已经将分母和分子配平，天然完美对齐了 1.0 白点
    return (max_x * (6.2 * max_x + vec3(0.5))) / (max_x * (6.2 * max_x + vec3(1.7)) + vec3(0.06));
}

//Reinhard 色调映射 (最经典的线性拉伸压缩)
// 1. 标准 Reinhard 算法
vec3 ReinhardTonemap(vec3 x) {
    return x / (vec3(1.0) + x);
}

// 2. 改进版 Reinhard (支持指定白点 W，超过 W 强行截断为纯白)
vec3 ReinhardTonemapExtended(vec3 x, float whitePoint) {
    vec3 numerator = x * (vec3(1.0) + (x / (whitePoint * whitePoint)));
    return numerator / (vec3(1.0) + x);
}

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}
// 色调映射，如果是正向渲染（Forward Rendering）且没有复杂的后处理，
// 可以直接在计算完所有光源的 Fragment Shader 末尾调用 tonemap 函数，然后输出
// 单独开一个（或多个）后处理 Pass
// 先将所有光照结果输出到一个 16位浮点格式（如 VK_FORMAT_R16G16B16A16_SFLOAT）的 HDR 纹理中
// 单独开一个全屏 Quad（或者 Compute Shader）作为后处理 Pass，读取这个 HDR 纹理并应用 Tone Mapping
// 可以轻松地在光照和 Tone Mapping 之间插入
// Bloom、Motion Blur（运动模糊）、SSR（屏幕空间反射）、TAA（时间性抗锯齿）
// 这些特效都必须在 HDR 线性空间下计算才物理正确
vec4 tonemap(vec4 color, float exposure, float gamma)
{
    vec3 outcol = Uncharted2Tonemap(color.rgb * exposure);
    // 计算白点修正值，确保输入为 W 时的亮度能完美映射为 1.0
    outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
    //选择 11.2f 是 真实的物理世界亮度以及电影胶片曲线反复调试出来的“白点阀值（Linear White Point）”
    return vec4(pow(outcol, vec3(1.0f / gamma)), color.a);
}

// 需要 与 PBR_component 布局相同
struct ShaderMaterial {
    vec4 baseColorFactor;
    vec4 emissiveFactor;

    float metallicFactor;
    float roughnessFactor;
    float occlusionStrength;
    float alphaCutoff;
    float ior;                // 折射率

    uint doubleSided; // 是否开始背面剪裁， 叶子、旗帜、纸张等超薄物体 需要为 true
    uint alphaMode;           // 有三个值
//                              OPAQUE (不透明 - 默认)
//                              MASK  基于 alphaCutoff 阈值进行“全有或全无”的硬切
//                              BLEND (混合/半透明)
// 需要一个为全为一的贴图，也就是纯白的贴图
    uint baseColorTexture; // 基础颜色 贴图
    uint normalTexture; //

    uint emissiveTexture; // 自发光 贴图
    uint ORM_Texture; // Occlusion, Roughness, Metallic

    uint pad_1;
// 下面这两个有什么用？
//    vec4 diffuseFactor;
//    vec4 specularFactor;
// 那么总共的字节数 是 16 + 16 + 16 + 16 + 16  = 80 字节
};


struct AABB_box {
    vec4 centroid_points;
    vec4 direction_intervals;
};



bool IsAABBInFrustum(Frustum frustum, AABB_box box)
{
    for (int i = 0; i < 6; ++i)
    {
        float projectedRadius = dot(box.direction_intervals, abs(frustum.frustum_planes[i]));
        float distanceToCenter = dot(box.centroid_points, frustum.frustum_planes[i]);
        if (distanceToCenter < -projectedRadius)
        {
            return false;
        }
    }
    return true;
}




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


struct Light {
    vec4 pos;
    vec4 rotate;
    vec4 color;
    float intensity;
    float range;
    float angle_scale;
    float angle_offset;
};

// 在 GLSL 中使用四元数旋转默认的 -Z 轴向量
vec3 quaternion_transform(vec4 q, vec3 v) {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}



vec3 Directional_light(Light light, vec3 world_pos, out vec3 L) {
    L = -normalize(light.rotate.xyz);
    return light.color.rgb * light.intensity;
}

vec3 Spot_light(Light light, vec3 world_pos, out vec3 L) {
    vec3 pos_err = light.pos.xyz - world_pos;
    L = -normalize(pos_err);
    float distanceSq = dot(pos_err, pos_err);
    float rangeSq = light.range * light.range;
    float factor = distanceSq / rangeSq;
    float smoothFactor = clamp(1.0f - factor * factor, 0.0f, 1.0f);
    float result = (smoothFactor * smoothFactor) / max(distanceSq, 0.0001f);
    return light.color.rgb * light.intensity * result;

}



// NdotL 可以替换 为其他的吗？
//  float cd 灯光夹角余弦
vec3 Point_light(Light light, vec3 world_pos, out vec3 L) {
    vec3 pos_err = world_pos - light.pos.xyz;
    L = -normalize(pos_err);
    float distanceSq = dot(pos_err, pos_err);
    vec3 defaultDir = vec3(0.0, 0.0, -1.0);
    vec3 light_direction = normalize(light.rotate.xyz);
    float cd = dot(light_direction, -L);
    float rangeSq = light.range * light.range;
    float factor = distanceSq / rangeSq;
    float smoothFactor = clamp(1.0f - factor * factor, 0.0f, 1.0f);
    float result = (smoothFactor * smoothFactor) / max(distanceSq, 0.0001f);
    float attenuation = clamp(cd * light.angle_scale + light.angle_offset, 0.0, 1.0);
    return light.color.rgb * light.intensity * result * attenuation;
}


float hash(int xy) {
    uint x = uint(xy);
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = (x >> 16u) ^ x;
    return float(x) / 4294967295.0;
}

float hash(uint xy) {
    uint x = xy;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = (x >> 16u) ^ x;
    return float(x) / 4294967295.0;
}



// Normal Distribution function --------------------------------------
// 在当前材质粗糙度下，有多少比例的“微表面”刚好把光线反射到你的眼睛里
// 本质的结果是一个 概率 的近似
float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denom * denom);
}


// Geometric Shadowing function --------------------------------------
// Epic Games 在 2013 年发表 UE4 PBR  为了绝对追求计算速度而做的一种近似截断
// 在当前粗糙度下，因为微表面自身的“凹凸不平”，有多少光线会被旁边的微小结构给“遮挡”住
// 微观的情况下 , 宏观的 还需要重新计算
float G_SchlicksmithGGX(float roughness, float dotNV, float dotNL)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

/**
* 上面的 G_SchlicksmithGGX 和 下面 G 不是来自于 一个 几何遮蔽模型
* 上面的 是 Smith-Schlick 模型
* 下面的 是 Smith-GGX（Height-Correlated，高度相关） 模型
*
**/


/**
* Correlated 相关联
* 追求更精确的物理表现
* Eric Heitz 在 2014 年指出：微表面是有高度的，遮蔽和掩膜高度相关
* 包含 Λ 的高度耦合分式（带根号） 的 结果
**/
float V_SmithGGXCorrelated(float roughness, float dotNV, float dotNL) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    float a2 = roughness * roughness;
    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaV = dotNL * sqrt((dotNV - a2 * dotNV) * dotNV + a2);
    float lambdaL = dotNV * sqrt((dotNL - a2 * dotNL) * dotNL + a2);
    // 0.0000077 = nextafter(0.5 / MEDIUMP_FLT_MAX, 1.0) in fp16, so we don't overflow
    float v = PREVENT_DIV0(0.5, lambdaV + lambdaL, 0.0000077);
    // #define PREVENT_DIV0(n, d, magic)   ((n) / max(d, magic))
    // a2=0 => v = 1 / 4*NoL*NoV   => min=1/4, max=+inf
    // a2=1 => v = 1 / 2*(NoL+NoV) => min=1/4, max=+inf
    return v;
}

float V_SmithGGXCorrelated_Fast(float roughness, float NoV, float NoL) {
    // Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
    // 0.0000077 = nextafter(0.5 / MEDIUMP_FLT_MAX, 1.0) in fp16, so we don't overflow
    float v = PREVENT_DIV0(0.5, mix(2.0 * NoL * NoV, NoL + NoV, roughness), 0.0000077);
    return v;
}



// Fresnel function ----------------------------------------------------
// 光线在两种不同介质的交界面上，有多少比例的光被【镜面反射】（Specular）回去了
// 在真实世界中，一个物体的反射率并不是固定的，而是随着你的观察角度（视角）变化而变化：
//     垂直看（反射弱）：当你垂直看着一汪清水或一块玻璃时（入射角为 0°），
//                    你能轻易看清甚至穿透它们，此时镜面反射最弱（水面只有约 2% 的光被反射）。
//     斜着看（反射强）：当你几乎平行于水面或侧面看玻璃边缘时（掠射角，入射角接近 90°），
//                    水面或玻璃会变成一面完美的镜子，此时镜面反射率暴增到 100%。
// Fresnel-Schlick 公式计算的，正是这种“越往边缘看，镜面反射越强烈”的动态比例。
vec3 F_Schlick(float cosTheta, vec3 baseColor, float metallic)
{
    vec3 F0 = mix(vec3(0.04), baseColor, metallic); //  基础反射率
    vec3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0, 1), 5.0);
    return F;
}


struct PixelParams {
    vec3 diffuseColor;
    float perceptualRoughness;
    float perceptualRoughnessUnclamped;
    vec3 f0;
#if defined(MATERIAL_HAS_SPECULAR_COLOR_FACTOR) || defined(MATERIAL_HAS_SPECULAR_FACTOR)
    float f90;
    float specular;
    vec3 specularColor;
#endif
    float roughness;
    vec3 dfg;
    vec3 energyCompensation;

#if defined(MATERIAL_HAS_CLEAR_COAT)
    float clearCoat;
    float clearCoatPerceptualRoughness;
    float clearCoatRoughness;
#endif

#if defined(MATERIAL_HAS_SHEEN_COLOR)
    vec3 sheenColor;
#if !defined(SHADING_MODEL_CLOTH)
    float sheenRoughness;
    float sheenPerceptualRoughness;
    float sheenScaling;
    float sheenDFG;
#endif
#endif

#if defined(MATERIAL_HAS_ANISOTROPY)
    vec3 anisotropicT;
    vec3 anisotropicB;
    float anisotropy;
#endif

#if defined(SHADING_MODEL_SUBSURFACE) || defined(MATERIAL_HAS_REFRACTION)
    float thickness;
#endif
#if defined(SHADING_MODEL_SUBSURFACE)
    vec3 subsurfaceColor;
    float subsurfacePower;
#endif

#if defined(SHADING_MODEL_CLOTH) && defined(MATERIAL_HAS_SUBSURFACE_COLOR)
    vec3 subsurfaceColor;
#endif

#if defined(MATERIAL_HAS_REFRACTION)
    float etaRI;
    float etaIR;
#if defined(MATERIAL_HAS_DISPERSION) && (REFRACTION_TYPE == REFRACTION_TYPE_SOLID)
    float dispersion;
#endif
    float transmission;
    float uThickness;
    vec3 absorption;
#endif
};



vec3 function_specular(float dotNV, float dotNL, float D, float G, vec3 F) {
    return D * F * G / (4.0 * dotNL * dotNV);
    // F  给出了 光滑的 情况下 反射到这个方向的能量
    // D 给出 F 之后 因为粗糙度 还有多少到 需要的方向
    // G 给出了 因为 微观 遮挡 还剩余多少

    // 光源的投影拉伸
    // dotNL 朗伯余弦定律（Lambert's Cosine Law）
    // 当一束手电筒的光垂直打在墙上时，光斑很小很亮；当手电筒斜着打在墙上时，光斑会被拉长、面积变大，导致单位面积内的光子数量（光照强度）变稀疏了
    // 分母上的 dotNL 作为一个修正项，就是为了抵消光线斜射时所带来的宏观表面积增大、光线被稀释的几何效应
    //
    // 视角的立体角变换
    // 当你从宏观去看一个表面时，你眼睛（或者相机像素）所看到的，实际上是一个宏观的平坦区域。
    // 但在这个区域内部，微表面是高低起伏的，它们的真实总面积其实比宏观面积大得多。
    // 微表面理论里的 D 计算的是微观空间下的微表面面积密度（相对于微观总面积的比例）。但是，我们最终是要把它画在屏幕的宏观像素上
    // 从你的眼睛（视角 V ）看过去，宏观表面和微观表面之间存在一个空间立体角（Solid Angle）的几何投影转换。
    // dotNV 就是用来完成这个“微观空间 --> 宏观视角”转换的缩放因子。

    // 法线与半程向量非常接近时会出现高光                 形成一个极亮、极小的高光点（类似太阳在镜子里的倒影）
    // dotNL  dotNV 都接近 90度时，也就是值都接近于零时   在物体轮廓边缘产生一道极亮的“银边”
}




vec3 get_c_diffusen(vec3 base_color, float metallic) {
    vec3 c_diffuse = base_color.rgb * (vec3(1.0) - 0.04) * (1.0 - metallic);
    return c_diffuse;
}
vec3 get_diffuse_contribution(vec3 base_color, float metallic) {
    vec3 c_diffuse = get_c_diffusen(base_color, metallic);
    vec3 diffuse_contribution = c_diffuse / 3.14159265359; // 基础 Lambert 漫反射
    return diffuse_contribution;
}
struct SphericalHarmonics {
    vec3 data[9];
};

vec3 Irradiance_SphericalHarmonics(const vec3 n, SphericalHarmonics SH) {
    vec3 sphericalHarmonics = SH.data[0];

    sphericalHarmonics +=
    SH.data[1] * (n.y)
    + SH.data[2] * (n.z)
    + SH.data[3] * (n.x);


    sphericalHarmonics +=
    SH.data[4] * (n.y * n.x)
    + SH.data[5] * (n.y * n.z)
    + SH.data[6] * (3.0 * n.z * n.z - 1.0)
    + SH.data[7] * (n.z * n.x)
    + SH.data[8] * (n.x * n.x - n.y * n.y);


    return max(sphericalHarmonics, 0.0);
}

uint MortonCode2(uint x)
{
    x &= 0x0000ffff;
    x = (x ^ (x << 8)) & 0x00ff00ff;
    x = (x ^ (x << 4)) & 0x0f0f0f0f;
    x = (x ^ (x << 2)) & 0x33333333;
    x = (x ^ (x << 1)) & 0x55555555;
    return x;
}

// Encodes two 16-bit integers into one 32-bit morton code
uint MortonEncode(uvec2 Pixel)
{
    uint Morton = MortonCode2(Pixel.x) | (MortonCode2(Pixel.y) << 1);
    return Morton;
}

uint ReverseMortonCode2(uint x)
{
    x &= 0x55555555;
    x = (x ^ (x >> 1)) & 0x33333333;
    x = (x ^ (x >> 2)) & 0x0f0f0f0f;
    x = (x ^ (x >> 4)) & 0x00ff00ff;
    x = (x ^ (x >> 8)) & 0x0000ffff;
    return x;
}

// Decodes one 32-bit morton code into two 16-bit integers
uvec2 MortonDecode(uint Morton)
{
    uvec2 Pixel = uvec2(ReverseMortonCode2(Morton), ReverseMortonCode2(Morton >> 1));
    return Pixel;
}

uint MortonCode3(uint x)
{
    x &= 0x000003ff;
    x = (x ^ (x << 16)) & 0xff0000ff;
    x = (x ^ (x << 8)) & 0x0300f00f;
    x = (x ^ (x << 4)) & 0x030c30c3;
    x = (x ^ (x << 2)) & 0x09249249;
    return x;
}

uint ReverseMortonCode3(uint x)
{
    x &= 0x09249249;
    x = (x ^ (x >> 2)) & 0x030c30c3;
    x = (x ^ (x >> 4)) & 0x0300f00f;
    x = (x ^ (x >> 8)) & 0xff0000ff;
    x = (x ^ (x >> 16)) & 0x000003ff;
    return x;
}

// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================================
	PathTracingRandomSequence.ush: Reference path tracing
===============================================================================================*/




#define RANDSEQ_PURERANDOM    0
#define RANDSEQ_OWENSOBOL    1
#define RANDSEQ_LATTICE     2

// Select a default if none was specified
#ifndef RANDSEQ
#define RANDSEQ            RANDSEQ_OWENSOBOL
#endif

#ifndef RANDSEQ_RANDOMIZED
#define RANDSEQ_RANDOMIZED        1 // 1 to randomize per pixel, 0 to share the same sequence in all pixels (useful to benchmark coherent sampling)
#endif

#ifndef RANDSEQ_ERROR_DIFFUSION
#define RANDSEQ_ERROR_DIFFUSION 1 // Enabled by default, only works when using RANDSEQ_OWENSOBOL or RANDSEQ_LATTICE
#endif

#ifndef RANDSEQ_UNROLL_SOBOL
#define RANDSEQ_UNROLL_SOBOL 1 // Use unrolled Sobol loop such that table can be inline constants.
#endif


#ifndef RANDSEQ_SFC_TEXTURE
#define RANDSEQ_SFC_TEXTURE 0 // 0 uses a Hilbert curve (ALU heavy) while 1 uses a precomputed texture (must be passed as a shader parameter)
#endif

// This hash mixes bits at the theoretical bias limit
uint PerfectIntegerHash(uint x)
{
    // https://nullprogram.com/blog/2018/07/31/
    // exact bias: 0.020888578919738908
    x ^= x >> 17;
    x *= 0xed5ad4bbu;
    x ^= x >> 11;
    x *= 0xac4c1b51u;
    x ^= x >> 15;
    x *= 0x31848babu;
    x ^= x >> 14;
    return x;
}

// High quality integer hash - this mixes bits almost perfectly
uint StrongIntegerHash(uint x)
{
    // From https://github.com/skeeto/hash-prospector
    // Current best hash in this form: https://github.com/skeeto/hash-prospector/issues/19#issuecomment-1105792898
    // bias = 0.10734781817103507
    x ^= x >> 16;
    x *= 0x21f0aaad;
    x ^= x >> 15;
    x *= 0xf35a2d97;
    x ^= x >> 15;
    return x;
}

// Base must be a prime number
float Halton(uint Index, uint Base)
{
    float r = 0.0, f = 1.0;
    float BaseInv = 1.0 / Base;
    while (Index > 0)
    {
        f *= BaseInv;
        r += f * (Index % Base);
        Index /= Base;
    }
    return r;
}

uint FastOwenScramblingCore(uint Index, uint Seed)
{
    #if 0
    // reference implementation - scramble bits one at a time

    // loop below does not require pre-inverted bits, so undo the inversion assumed in the optimized case below
    Index = bitfieldReverse(Index);
    for (uint Mask = 1u << 31, Input = Index; Mask; Mask >>= 1) {
        Seed = PerfectIntegerHash(Seed); // randomize state
        Index ^= Seed & Mask;  // flip output (depending on state)
        Seed ^= Input & Mask;  // flip state  (depending on input)
    }
    return Index;
    #else
    // This simple tweak appears to be nearly as good as the reference, while being cheaper to compute
    Index ^= Index * 0xe0705d72u;
    Index += Seed;
    Seed ^= Seed >> 16;
    Index *= Seed | 1;
    #endif
    return Index;
}

uint FastOwenScrambling(uint Index, uint Seed)
{
    Index = FastOwenScramblingCore(Index, Seed);
    // Undo the reverse so that we get left-to-right scrambling
    // thereby emulating owen-scrambling
    return bitfieldReverse(Index);
}

// Given a point (X,Y) on the unit square [0,2^B)^2, return the index along the hilbert curve in [0,2^2B)
// Because this function only loops over the bits it uses, there is no need to mask out bits beyond the
// bit count, the pattern will simply tile. This is meant to be a fallback if you don't want/need to pass in
// the space filling curve texture.
uint SFCInverse(uint X, uint Y, int NumBits)
{
    // https://github.com/hcs0/Hackers-Delight/blob/master/hilbert/hil_s_from_xy.c.txt
    uint HilbertIndex = 0, HilbertState = 0;
    for (int i = NumBits; i > 0u;) {
        i--;
        uint xi = (X >> i) & 1;
        uint yi = (Y >> i) & 1;
        uint Row = 8 * HilbertState + 4 * xi + 2 * yi;
        HilbertIndex = HilbertIndex * 4 + ((0x361E9CB4u >> Row) & 3);
        HilbertState = (0x8FE65831u >> Row) & 3;
    }
    return HilbertIndex;
}

uint EvolveSobolSeed(inout uint Seed)
{
    // LCG parameters from
    // "Computationally Easy, Spectrally Good Multipliers for Congruential Pseudorandom Number Generators"
    //   Guy Steele, Sebastiano Vigna
    //  Journal of Software: Practice and Experience, Volume 52, Issue 2, Feb 2022
    return Seed = Seed * 0x915f77f5u + 0x93d765ddu;
}

uvec4 SamplerCore(uint SampleIndex, inout uint Seed)
{
    #if RANDSEQ == RANDSEQ_PURERANDOM
    uvec4 Result = uvec4(StrongIntegerHash(Seed + 0),
            StrongIntegerHash(Seed + 1),
            StrongIntegerHash(Seed + 2),
            StrongIntegerHash(Seed + 3));
    Seed += 4;
    return Result;
    #elif RANDSEQ == RANDSEQ_OWENSOBOL
    // first scramble the index to decorelate from other 4-tuples
    uint SobolIndex = FastOwenScrambling(SampleIndex, EvolveSobolSeed(Seed));
    // now get Sobol' point from this index
    uvec4 Result = uvec4(SobolIndex, SobolIndex, 0, 0);
    // "SZ Sequences: Binary-Based (0,2^q)-Sequences"
    //  Abdalla G. M. Ahmed, Matt Pharr, Victor Ostromoukhov, Hui Huang
    // 	  https://dl.acm.org/doi/epdf/10.1145/3763272
    //    Siggraph Asia 2025
    // Optimized evaluation of Dimension 2 via Equation 42
    Result.z = ((SobolIndex >> 1) & 0x50550550u) ^
    ((SobolIndex) & 0xaf5af5afu) ^
    ((SobolIndex << 1) & 0xa0aa0aa0u);
    Result.z ^= Result.z << 16;
    Result.z ^= (Result.z & 0x00FF00FF) << 8;
    Result.z ^= (Result.z & 0x0F0F0F0F) << 4;
    Result.z = ((Result.z >> 1) & 0x50550550u) ^
    ((Result.z) & 0x5fa5fa5fu) ^
    ((Result.z << 1) & 0xa4ea4ea4u) ^
    ((Result.z << 2) & 0xc84c84c8u) ^
    ((Result.z << 3) & 0x08808808u);
    Result.zw = uvec2(bitfieldReverse(Result.z));
    // Result.yw can be computed without iteration by starting with Result.xz
    // "An Implementation Algorithm of 2D Sobol Sequence Fast, Elegant, and Compact"
    // Abdalla Ahmed, EGSR 2024
    // See listing (19) in the paper
    // The code is different here because we want the output to be bit-reversed, but
    // the methodology is the same
    Result.yw ^= Result.yw >> 16;
    Result.yw ^= (Result.yw & 0xFF00FF00) >> 8;
    Result.yw ^= (Result.yw & 0xF0F0F0F0) >> 4;
    Result.yw ^= (Result.yw & 0xCCCCCCCC) >> 2;
    Result.yw ^= (Result.yw & 0xAAAAAAAA) >> 1;
    // finally scramble the points to avoid structured artifacts
    Result.x = FastOwenScrambling(Result.x, EvolveSobolSeed(Seed));
    Result.y = FastOwenScrambling(Result.y, EvolveSobolSeed(Seed));
    Result.z = FastOwenScrambling(Result.z, EvolveSobolSeed(Seed));
    Result.w = FastOwenScrambling(Result.w, EvolveSobolSeed(Seed));
    return Result;
    #elif RANDSEQ == RANDSEQ_LATTICE
    // Simplified algorithm using rank-1 lattice sequences. It avoids the extra call to reverse bits
    // Scrambling the _output_ does not appear to help, so we only need to scramble the index, leading to a very fast algorithm
    // Unfortunately, the resulting sequence does not have quite as good quality as Sobol in practice

    uint LatticeIndex = FastOwenScramblingCore(SampleIndex, EvolveSobolSeed(Seed));

    // Lattice parameters taken from:
    // Weighted compound integration rules with higher order convergence for all N
    // Fred J. Hickernell, Peter Kritzer, Frances Y. Kuo, Dirk Nuyens
    // Numerical Algorithms - February 2012
    return LatticeIndex * uvec4(1, 364981, 245389, 97823);
    #endif
}

/// A simple struct that encapsulates a random number sequence (extensible in dimension)
/// Once you create the struct, you can draw samples in the unit square (as either floats or bits) in any dimension from 1 to 4.
/// If you need more dimensions, you can repeat the calls, which will be guaranteed to be statistically independent from the previous
/// However the values from a single call (Get1D(), Get2D(), etc ...) should have nice stratification properties
/// If you need to draw N samples for a pixel, you need to create the struct N times.
struct RandomSequence
{
    uint SampleIndex; // index into the random sequence
    uint SampleSeed; // changes as we draw samples to reflect the change in dimension

};

// This initializes a multi-dimensional sampler for a particular pixel. The FrameIndex parameter should be set
// such that it changes per sample. See the function below when you know how many samples you plan on taking
// for a particular integral.
RandomSequence RandomSequenceCreate(uint PositionSeed, uint FrameIndex)
{
    // optionally disable randomization per pixel so that all pixels follow the same path in primary sample space
    #if RANDSEQ_RANDOMIZED == 0
    PositionSeed = 0;
    #endif

    RandomSequence RandSequence;

    #if RANDSEQ == RANDSEQ_PURERANDOM
    RandSequence.SampleIndex = 0; // not used
    RandSequence.SampleSeed = StrongIntegerHash(PositionSeed + StrongIntegerHash(FrameIndex));
    #elif RANDSEQ == RANDSEQ_OWENSOBOL || RANDSEQ == RANDSEQ_LATTICE
    // pre-compute bit reversal needed for FastOwenScrambling since this index doesn't change
    RandSequence.SampleIndex = bitfieldReverse(FrameIndex);
    // change seed to get a unique sequence per pixel
    RandSequence.SampleSeed = StrongIntegerHash(PositionSeed);
    #else
    #error "Unknown random sequence chosen in path tracer"
    #endif
    return RandSequence;
}


// This is an alternative initialization to the method above when you know how many samples you are planning on taking. This incorporates the
// TimeSeed0 (this should usually be the frame number when using all samples in one frame, or should be the frame on which the first sample was
// taken if taking one sample per frame like the path tracer).
RandomSequence RandomSequenceCreate(uvec3 PixelCoordAndFrame, uint SampleIndex, uint MaxSamples)
{
    RandomSequence RandSequence;

    #if RANDSEQ_ERROR_DIFFUSION == 1 && (RANDSEQ == RANDSEQ_OWENSOBOL || RANDSEQ == RANDSEQ_LATTICE)
    // The core idea is to use a single sobol pattern across the screen using a space filling curve to correlate nearby pixels
    // This achieves a much better error diffusion than the random sequence assignment of the method above
    // This algorithm is covered in the following paper:
    //   "Screen-Space Blue-Noise Diffusion of Monte Carlo Sampling Error via Hierarchical Ordering of Pixels"
    //   Siggraph Asia 2020
    //   http://abdallagafar.com/publications/zsampler/
    // Improvements made in this version are:
    //   - switching the z-order curve for a sierpinski curve
    //   - switching sobol points for owen-sobol points
    //   - recognizing that the hierarchical scrambling proposed in the original paper is equivalent to Owen-Scrambling of the index
    // Downsides of this method:
    //   - quality does not improve beyond the target sample count, making usage with adaptive sampling difficult
    //   - error-diffusion property is only achieved at the target sample count, earlier samples look random (sometimes slightly worse than random)

    uint TileID = SFCInverse(PixelCoordAndFrame.x, PixelCoordAndFrame.y, 8);

    #if 0
    // Combine frame/sample index into the spatial index in a way that diffuses the error spatially and temporally
    // However this appears to be worse overall, averaging successive frames does not converge quickly
    // instead there are artifacts resulting from the space filling curve being explored in linear order.
    TileID ^= FastOwenScrambling(bitfieldReverse(PixelCoordAndFrame.z), 0xcafef00du);
    #else
    TileID += PixelCoordAndFrame.z * 65536;
    #endif

    // Combine pixel-level and sample-level bits into the sample index (visible structure will be hidden by owen scrambling of the index)
    RandSequence.SampleIndex = bitfieldReverse(TileID * MaxSamples + SampleIndex);

    // progressive encodings (these kind of work, but quality is much worse)
    //RandSequence.SampleIndex = bitfieldReverse(MortonEncode(uint2(TimeSeed0, TileID)) * MaxSamples + SampleIndex);
    //RandSequence.SampleIndex = bitfieldReverse((MortonCode3(MaxSamples * TimeSeed0 + SampleIndex) * 4) | (MortonCode3(Y) * 2) | MortonCode3(X));
    //RandSequence.SampleIndex = bitfieldReverse(MortonEncode(uint2(MortonEncode(uint2(X, Y)), SampleIndex)));

    RandSequence.SampleSeed = 0; // always use the same sequence
    return RandSequence;
    #else
    // Error diffusion sampler is disabled, just use the simple init function instead
    return RandomSequenceCreate(PixelCoordAndFrame.x + PixelCoordAndFrame.y * 65536, PixelCoordAndFrame.z * MaxSamples + SampleIndex);
    #endif
}

RandomSequence Split(RandomSequence temp, uint Index, uint Num)
{
    RandomSequence SplitSequence;
    #if RANDSEQ == RANDSEQ_PURERANDOM
    SplitSequence.SampleIndex = 0;
    SplitSequence.SampleSeed = StrongIntegerHash(temp.SampleSeed * Num + Index);
    #else
    // Undo the built-in bitfieldReverse, stretch the index and re-apply it
    SplitSequence.SampleSeed = temp.SampleSeed;
    SplitSequence.SampleIndex = bitfieldReverse(bitfieldReverse(temp.SampleIndex) * Num + Index);
    #endif
    return SplitSequence;
}


// Legacy API - wrappers around the more convenient calls above, but kept for back-compatiblity
// Deprecated (5.7)
void RandomSequence_Initialize(inout RandomSequence RandSequence, uint PositionSeed, uint FrameIndex)
{
    RandSequence = RandomSequenceCreate(PositionSeed, FrameIndex);
}

void RandomSequence_Initialize(inout RandomSequence RandSequence, uvec2 PixelCoord, uint SampleIndex, uint FrameIndex, uint MaxSamples)
{
    RandSequence = RandomSequenceCreate(uvec3(PixelCoord, FrameIndex), SampleIndex, MaxSamples);
}

float RandomSequence_GenerateSample1D(inout RandomSequence RandSequence)
{
    return (SamplerCore(RandSequence.SampleIndex, RandSequence.SampleSeed).x >> 8) * 5.96046447754e-08;
}

vec2 RandomSequence_GenerateSample2D(inout RandomSequence RandSequence)
{
    return (SamplerCore(RandSequence.SampleIndex, RandSequence.SampleSeed).xy >> 8) * 5.96046447754e-08;
}

vec3 RandomSequence_GenerateSample3D(inout RandomSequence RandSequence)
{
    return (SamplerCore(RandSequence.SampleIndex, RandSequence.SampleSeed).xyz >> 8) * 5.96046447754e-08;
}

vec4 RandomSequence_GenerateSample4D(inout RandomSequence RandSequence)
{
    return (SamplerCore(RandSequence.SampleIndex, RandSequence.SampleSeed) >> 8) * 5.96046447754e-08;
}


// Return a random value in [0,2^32)^d
// All functions rely on the 4D variant and we rely on compiler to optimize out the dead code
// This also guarantees that the sequence will not "shift" if you change a call from one dimension to another because the seed will be changed in a consistent way in all cases
//uint Get1DBits() { return SamplerCore(SampleIndex, SampleSeed).x; }
//uint2 Get2DBits() { return SamplerCore(SampleIndex, SampleSeed).xy; }
//uint3 Get3DBits() { return SamplerCore(SampleIndex, SampleSeed).xyz; }
//uint4 Get4DBits() { return SamplerCore(SampleIndex, SampleSeed); }

// Convert the random value in [0,2^32)^d to a uniformly distributed float in [0,1)^d,
// taking care not to skew the distribution due to the non-uniform spacing of floats.
// Use the upper 24 bits to preserve the good properties of low-discrenpancy samplers.
// The floating point constant is 2^-24 (would be more nicely expressed with hex-floats which are not yet supported in HLSL)
// The implicit int to float cast here is guaranteed to be lossless because float can represent integers up to 2^24 exactly
//float Get1D() { return (Get1DBits() >> 8) * 5.96046447754e-08; }
//float2 Get2D() { return (Get2DBits() >> 8) * 5.96046447754e-08; }
//float3 Get3D() { return (Get3DBits() >> 8) * 5.96046447754e-08; }
//float4 Get4D() {  }

// Split the path into Num entries. The RandomSequence object returned can be used to keep sampling along the "Index" branch of the split
// Index must be a value in [0,Num), Num should be >0
//FRandomSequence Split(uint Index, uint Num);

vec4 lookup(in vec4 textureColor, in sampler2D lookupTable) {
    #ifndef LUT_NO_CLAMP
    textureColor = clamp(textureColor, 0.0, 1.0);
    #endif

    // Vulkan 中直接使用标准 float，去除 OpenGL ES 的 mediump 关键字
    float blueColor = textureColor.b * 63.0;

    vec2 quad1;
    quad1.y = floor(floor(blueColor) / 8.0);
    quad1.x = floor(blueColor) - (quad1.y * 8.0);

    vec2 quad2;
    quad2.y = floor(ceil(blueColor) / 8.0);
    quad2.x = ceil(blueColor) - (quad2.y * 8.0);

    vec2 texPos1;
    texPos1.x = (quad1.x * 0.125) + 0.5 / 512.0 + ((0.125 - 1.0 / 512.0) * textureColor.r);
    texPos1.y = (quad1.y * 0.125) + 0.5 / 512.0 + ((0.125 - 1.0 / 512.0) * textureColor.g);

    #ifdef LUT_FLIP_Y
    texPos1.y = 1.0 - texPos1.y;
    #endif

    vec2 texPos2;
    texPos2.x = (quad2.x * 0.125) + 0.5 / 512.0 + ((0.125 - 1.0 / 512.0) * textureColor.r);
    texPos2.y = (quad2.y * 0.125) + 0.5 / 512.0 + ((0.125 - 1.0 / 512.0) * textureColor.g);

    #ifdef LUT_FLIP_Y
    texPos2.y = 1.0 - texPos2.y;
    #endif

    // 关键修改：将 texture2D 升级为符合 Vulkan 标准的 texture
    vec4 newColor1 = texture(lookupTable, texPos1);
    vec4 newColor2 = texture(lookupTable, texPos2);

    vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
    return newColor;
}

// 适用于 256x16 的长条形 LUT 采样 (16³ 尺寸)
vec3 ColorLookup2DSRGB(sampler2D lutTex, vec3 uvw)
{
    // lutSize 是 16.0
    float lutSize = 16.0;

    // 1. 将 0~1 的 B 通道重新映射到 0 ~ 15 的切片索引范围
    float rwb = uvw.z * (lutSize - 1.0);

    // 2. 边缘对齐与微调 (防止色彩越界串线)
    float uOffset = 1.0 / (256.0);
    float vOffset = 1.0 / (16.0);

    // 3. 计算在当前单张切片内的 R 和 G 归一化内聚坐标
    vec2 quadUV;
    quadUV.x = uvw.x * (1.0 / lutSize - uOffset) + (uOffset * 0.5);
    quadUV.y = uvw.y * (1.0 - vOffset) + (vOffset * 0.5);

    // 4. 加上 B 轴切片带来的横向 X 轴位移
    float zSliceLow = floor(rwb);
    float zSliceHigh = ceil(rwb);

    vec2 uvLow = vec2(quadUV.x + zSliceLow / lutSize, quadUV.y);
    vec2 uvHigh = vec2(quadUV.x + zSliceHigh / lutSize, quadUV.y);

    // 5. 采样并按蓝色残差插值
    vec3 colorLow = texture(lutTex, uvLow).rgb;
    vec3 colorHigh = texture(lutTex, uvHigh).rgb;

    return mix(colorLow, colorHigh, fract(rwb));
}

// Z 值是负数
vec3 get_view_pos(vec2 uv, float depth, mat4 invProjection){
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    float x = invProjection[0][0] * clipPos.x;
    float y = invProjection[1][1] * clipPos.y;
    float z = EIGEN_INDEX(invProjection,2,2) * clipPos.z + EIGEN_INDEX(invProjection,2,3);
    float w = EIGEN_INDEX(invProjection,3,2) * clipPos.z + EIGEN_INDEX(invProjection,3,3);
    return vec3(x, y, z) / w;
}

/**
 * 计算暗角因子
 * @param uv           当前像素的归一化 UV 坐标 [0, 1]
 * @param intensity    暗角强度（通常 0.5 - 2.0）
 * @param smoothness   边缘平滑度（通常 0.1 - 1.0）
 * @return             颜色的缩放系数（乘到原颜色上）
 */
vec3 apply_vignette(vec3 color, vec2 uv, float intensity, float smoothness) {
    // 1. 将 UV 转换到 [-0.5, 0.5] 空间（中心点为 0）
    vec2 d = (uv - 0.5) * 2.0;

    // 2. 计算当前点到中心的距离平方（或者四次方，让中心更亮）
    float dist = dot(d, d);

    // 3. 使用 smoothstep 计算暗角衰减
    float vignette = smoothstep(1.0, smoothness, dist * intensity);

    // 4. 混合原图颜色
    return color * vignette;
}


/**
 * 计算镜头色差 / 色散效果
 * @param sceneTex     输入的场景原始颜色纹理
 * @param texSampler   采样器
 * @param uv           当前像素的归一化 UV 坐标 [0.0, 1.0]
 * @param rOffset      红光偏移系数（通常为正数，如 0.005）
 * @param bOffset      蓝光偏移系数（通常为负数，如 -0.005）
 * @return             计算色差后的 RGB 颜色值
 */
vec3 apply_chromatic_aberration(sampler2D sceneTex, vec2 uv, float rOffset, float bOffset) {
    // 1. 计算当前点到屏幕中心的向量和距离（边缘处色散越严重）
    vec2 dir = uv - 0.5;
    float dist = length(dir);

    // 2. 根据到中心的距离，对红蓝通道进行不同程度的 UV 偏移
    vec2 uvR = uv + dir * (dist * rOffset);
    vec2 uvG = uv; // 绿光折射率居中，作为基准保持不动
    vec2 uvB = uv + dir * (dist * bOffset);

    // 3. 分通道采样纹理并组合
    float r = texture(sceneTex, uvR).r;
    float g = texture(sceneTex, uvG).g;
    float b = texture(sceneTex, uvB).b;

    return vec3(r, g, b);
}

/**
 * 手动对 image2D 进行双线性插值采样
 * @param img     输入的 image2D
 * @param uv      归一化的浮点数 UV 坐标 [0.0, 1.0]
 * @return        插值后的 vec4 颜色
 */
vec4 texture_image2d_bilinear(image2D img, vec2 uv) {
    vec2 imgSize = vec2(imageSize(img));

    // 1. 将 0-1 的 UV 映射到 0-Size 的像素坐标空间
    // 减去 0.5 是因为像素中心在 (0.5, 0.5)
    vec2 texelCoord = uv * imgSize - 0.5;

    // 2. 找到左上角最近的整数像素坐标
    ivec2 baseCoord = ivec2(floor(texelCoord));

    // 3. 计算浮点数权重（当前点距离左上角像素中心的距离）
    vec2 f = texelCoord - floor(texelCoord); // 部分 GLSL 环境若无 frac 可用 texelCoord - floor(texelCoord)

    // 4. 边界处理：防止 ivec2 + 1 溢出图像边界
    ivec2 maxCoord = ivec2(imgSize) - 1;
    ivec2 c00 = clamp(baseCoord, ivec2(0), maxCoord);
    ivec2 c10 = clamp(baseCoord + ivec2(1, 0), ivec2(0), maxCoord);
    ivec2 c01 = clamp(baseCoord + ivec2(0, 1), ivec2(0), maxCoord);
    ivec2 c11 = clamp(baseCoord + ivec2(1, 1), ivec2(0), maxCoord);

    // 5. 读取邻域 4 个像素的颜色
    vec4 p00 = imageLoad(img, c00);
    vec4 p10 = imageLoad(img, c10);
    vec4 p01 = imageLoad(img, c01);
    vec4 p11 = imageLoad(img, c11);

    // 6. 进行双线性插值（先水平，再垂直）
    vec4 row0 = mix(p00, p10, f.x);
    vec4 row1 = mix(p01, p11, f.x);
    return mix(row0, row1, f.y);
}

/**
 * 基于 image2D 的镜头色差计算
 */
vec3 apply_chromatic_aberration_image(image2D sceneTex, vec2 uv, float rOffset, float bOffset) {
    vec2 dir = uv - 0.5;
    float dist = length(dir);

    // 根据到中心的距离，对红蓝通道进行不同程度的 UV 偏移
    vec2 uvR = uv + dir * (dist * rOffset);
    vec2 uvG = uv;
    vec2 uvB = uv + dir * (dist * bOffset);

    // 调用上面实现的手动双线性采样
    float r = texture_image2d_bilinear(sceneTex, uvR).r;
    float g = texture_image2d_bilinear(sceneTex, uvG).g;
    float b = texture_image2d_bilinear(sceneTex, uvB).b;

    return vec3(r, g, b);
}

/**
 * 计算雾的混合因子
 * @param distance   像素到相机的实际距离（或垂直深度）
 * @param fogStart   线性雾起始距离
 * @param fogEnd     线性雾结束距离
 * @param density    指数雾的密度系数  density (雾密度，即 fogColor.a)：通常是一个非常小的正数。
 *                       清晨/薄雾 (Light Mist)：0.005 到 0.01
 *                       普通大世界 (Standard World)：0.015 到 0.03
 *                       浓雾/大烟雾 (Heavy Fog)：0.05 到 0.1
 * @param fogType    0: 线性雾, 1: 指数雾, 2: 指数平方雾
 * @return           返回 [0.0, 1.0] 之间的因子。0.0 表示完全被雾覆盖，1.0 表示没有雾。
 */
float get_fog_factor(float distance, float fogStart, float fogEnd, float density, uint fogType) {
    float fogFactor = 1.0;

    if (fogType == 0) {
        // 线性雾 (Linear Fog)
        fogFactor = (fogEnd - distance) / (fogEnd - fogStart);
    }
    else if (fogType == 1) {
        // 指数雾 (Exponential Fog)
        fogFactor = exp(-density * distance);
    }
    else if (fogType == 2) {
        // 指数平方雾 (Exponential Squared Fog)
        fogFactor = exp(-pow(density * distance, 2.0));
    }

    // 强制限制在 [0.0, 1.0] 范围内，防止距离超出范围时计算出负数或大于 1 的值
    return clamp(fogFactor, 0.0, 1.0);
}


float get_noise_iq(uvec2 p, uint time_seed) {
    // 混入时间做种子
    uint qx = p.x ^ time_seed;
    uint qy = p.y ^ time_seed;
    // 经典的异或位移打散
    uint hx = qx * 1103515245u;
    uint hy = qy * 1103515245u;

    hx = (hx ^ (hx >> 15u)) * 1103515245u;
    hy = (hy ^ (hy >> 15u)) * 1103515245u;

    uint h = hx ^ hy;
    return float(h ^ (h >> 16u)) * (1.0 / 4294967295.0);
}

float get_noise_v2(uvec2 p, uint time_seed) {
    // 1. 🔥 将 2D 屏幕坐标与时间种子混合为一个 3D 整数向量
    uvec3 v = uvec3(p, time_seed);

    // 2. 🔥 大厂标准的多级“三向交叉乘法 ＋ 异或折叠” (PCG/Murmur3 的核心变种)
    // 强制让时间种子的变化直接以高频乘法的形式扩散到空间坐标的每一个 Bit 上
    v = v * 1664525u + 1013904223u;

    v.x += v.y * v.z;
    v.y += v.z * v.x;
    v.z += v.x * v.y;

    // 中间插入右移异或，将高位的随机性强行灌入低位
    v ^= v >> 16u;

    v.x += v.y * v.z;
    v.y += v.z * v.x;
    v.z += v.x * v.y;

    // 3. 映射到 [0.0, 1.0] 的浮点数
    return float(v.x) * (1.0 / 4294967295.0);
}

/**
 * 全通用 Vulkan 深度线性化函数
 * @param depth        从 Vulkan 深度图中采样出来的非线性 depth 值 [0, 1]
 * @param Projection   C++ 端传进来的原始投影矩阵（不管是常规还是 Reversed-Z，一网打尽）
 * @return             返回绝对真实的 View Space 垂直物理距离（米） Position.z 是一个负数
 */
float linearize_depth_vulkan_universal(float depth, mat4 Projection) {
    // 直接提取矩阵中掌管 NDC 到 ViewSpace 深度缩放与偏移的两大核心系数
    float P22 = EIGEN_INDEX(Projection, 2,2);
    float P32 = EIGEN_INDEX(Projection, 3,2);
    // 不管是常规 Z 还是 Reversed-Z，甚至包括“无限远景 Reversed-Z”，
    // 它们在代数化简后，在 GPU 硬件层面全都能完美收敛到这个极简的分式中！
    return P32 / (depth - P22);
}

float linearize_depth_vulkan(sampler2D depthImg, ivec2 texelCoord, mat4 Projection) {
    // 现代 Vulkan [0, 1] 深度对应的标准线性化公式
    float z = texelFetch(depthImg, texelCoord, 0).r;
    return linearize_depth_vulkan_universal(z, Projection);
}


#endif // COMMOM_FUNCTION_AND_STRUCT_INCLUDED