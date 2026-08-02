// 这里

struct VkDrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
};
const float PI = 3.14159265359;


struct Frustum {
    vec4 frustum_planes[6];
};

#ifdef TARGET_MOBILE
#define PREVENT_DIV0(n, d, magic)   ((n) / max(d, magic))
#else
#define PREVENT_DIV0(n, d, magic)   ((n) / (d))
#endif



// 需要 与 PBR_component 布局相同
struct ShaderMaterial {
    vec4 baseColorFactor;
    vec4 emissiveFactor;

    float metallicFactor;
    float roughnessFactor;
    float occlusionStrength;
    float alphaCutoff;

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
    uint pad_2;
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
