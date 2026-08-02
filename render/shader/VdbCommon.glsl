#define PNANOVDB_GLSL
#define PNANOVDB_ADDRESS_32
#include "PNanoVDB.h"


#ifndef NANOVDB_VDB_COMMON_H_HAS_BEEN_INCLUDED
#define NANOVDB_VDB_COMMON_H_HAS_BEEN_INCLUDED


struct Segment
{
    pnanovdb_vec3_t Start;
    pnanovdb_vec3_t End;
};

struct VdbRay
{
    pnanovdb_vec3_t Origin;
    pnanovdb_vec3_t Direction;
    float TMin;
    float TMax;
};

struct HeterogenousMedium
{
    float densityScale;
    float densityMin;
    float densityMax;
    float anisotropy;
    float albedo;
};

// From PathTracingPinholeCamera.ush
//float2 PinholeRandomSample(inout RandomSequence RandSequence)
//{
//    float2 AAJitter = RandomSequence_GenerateSample2D(RandSequence);
//
//    // importance sample a gaussian kernel with variable sigma
//    float3 Disk = ConcentricDiskSamplingHelper(AAJitter);
//    float FilterWidth = 2.0; // TODO: user defined FilterWidth (cf PathTracingData.FilterWidth)
//    float Sigma = FilterWidth / 6.0; // user-provided width covers +/-3*Sigma
//    float R = min(Disk.z, 0.99999994); // avoid log(0) when R=1
//    AAJitter = 0.5 + Sigma * Disk.xy * sqrt(-2.0 * log(1.0 - R * R));
//
//    return AAJitter;
//}

struct VdbSampler
{
    pnanovdb_grid_handle_t Grid;
    pnanovdb_buf_t GridBuffer;
    pnanovdb_readaccessor_t Accessor;
    pnanovdb_uint32_t GridType;
    pnanovdb_root_handle_t Root;
};

VdbSampler InitVdbSampler(pnanovdb_buf_t buf)
{
    VdbSampler Sampler;
    Sampler.GridBuffer = buf;

    pnanovdb_address_t address;
    address.byte_offset = 0;
    Sampler.Grid.address = address;

    pnanovdb_buf_t root_buf = buf;
    pnanovdb_tree_handle_t tree = pnanovdb_grid_get_tree(Sampler.GridBuffer, Sampler.Grid);
    Sampler.Root = pnanovdb_tree_get_root(root_buf, tree);

    pnanovdb_readaccessor_init(Sampler.Accessor, Sampler.Root);

    Sampler.GridType = pnanovdb_grid_get_grid_type(Sampler.GridBuffer, Sampler.Grid);

    return Sampler;
}

//-----------------------------------------------------------------------------------------------------------
// NanoVDB Buffers
//-----------------------------------------------------------------------------------------------------------

float ReadValue(pnanovdb_coord_t ijk, pnanovdb_buf_t buf, pnanovdb_uint32_t grid_type, in out pnanovdb_readaccessor_t acc)
{
    pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT,
                                                                         buf,
                                                                         acc,
                                                                         PNANOVDB_REF(ijk));
    return pnanovdb_read_float(buf, address);
}

float ReadValue(pnanovdb_vec3_t pos, pnanovdb_buf_t buf, pnanovdb_uint32_t grid_type, pnanovdb_readaccessor_t acc)
{
    pnanovdb_coord_t ijk = pnanovdb_hdda_pos_to_ijk(pos);
    return ReadValue(ijk, buf, grid_type, acc);
}

vec3 ReadValueVec3f(pnanovdb_coord_t ijk, pnanovdb_buf_t buf, pnanovdb_uint32_t grid_type, pnanovdb_readaccessor_t acc)
{
    pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(grid_type, buf, acc, ijk);

    return vec3(pnanovdb_read_float(buf, pnanovdb_address_offset(address, 0u)),
    pnanovdb_read_float(buf, pnanovdb_address_offset(address, 4u)),
    pnanovdb_read_float(buf, pnanovdb_address_offset(address, 8u)));
}

vec4 ReadValueVec4f(pnanovdb_coord_t ijk, pnanovdb_buf_t buf, pnanovdb_uint32_t grid_type, pnanovdb_readaccessor_t acc)
{
    pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(grid_type, buf, acc, ijk);

    return vec4(
    pnanovdb_read_float(buf, pnanovdb_address_offset(address, 0u)),
    pnanovdb_read_float(buf, pnanovdb_address_offset(address, 4u)),
    pnanovdb_read_float(buf, pnanovdb_address_offset(address, 8u)),
    pnanovdb_read_float(buf, pnanovdb_address_offset(address, 12u)));
}

// Faster, but float32 only
float ReadValueFloat(pnanovdb_coord_t ijk, pnanovdb_buf_t buf, pnanovdb_readaccessor_t acc)
{
    pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT, buf, acc, ijk);
    return pnanovdb_read_float(buf, address);
}

float ReadValueFloat(pnanovdb_vec3_t pos, pnanovdb_buf_t buf, pnanovdb_readaccessor_t acc)
{
    pnanovdb_coord_t ijk = pnanovdb_hdda_pos_to_ijk(pos);
    return ReadValueFloat(ijk, buf, acc);
}

// NearestNeighbor, Point sampling
float ReadValue(float Step, VdbRay ray, pnanovdb_buf_t buf, pnanovdb_uint32_t grid_type, pnanovdb_readaccessor_t acc)
{
    pnanovdb_vec3_t pos = pnanovdb_hdda_ray_start(ray.Origin, Step, ray.Direction);
    return ReadValue(pos, buf, grid_type, acc);
}

float TrilinearSampling(pnanovdb_vec3_t pos, pnanovdb_buf_t buf, pnanovdb_uint32_t grid_type, pnanovdb_readaccessor_t acc)
{
    pnanovdb_coord_t ijk = pnanovdb_hdda_pos_to_ijk(pos);
    pnanovdb_vec3_t uvw = pos - ijk;

    float Values[2][2][2];
    Values[0][0][0] = ReadValue(ijk + pnanovdb_coord_t(0, 0, 0), buf, grid_type, acc);
    Values[0][0][1] = ReadValue(ijk + pnanovdb_coord_t(0, 0, 1), buf, grid_type, acc);
    Values[0][1][1] = ReadValue(ijk + pnanovdb_coord_t(0, 1, 1), buf, grid_type, acc);
    Values[0][1][0] = ReadValue(ijk + pnanovdb_coord_t(0, 1, 0), buf, grid_type, acc);
    Values[1][0][0] = ReadValue(ijk + pnanovdb_coord_t(1, 0, 0), buf, grid_type, acc);
    Values[1][0][1] = ReadValue(ijk + pnanovdb_coord_t(1, 0, 1), buf, grid_type, acc);
    Values[1][1][1] = ReadValue(ijk + pnanovdb_coord_t(1, 1, 1), buf, grid_type, acc);
    Values[1][1][0] = ReadValue(ijk + pnanovdb_coord_t(1, 1, 0), buf, grid_type, acc);

    return mix(
        mix(
            mix(Values[0][0][0], Values[0][0][1], uvw[2]),
            mix(Values[0][1][0], Values[0][1][1], uvw[2]),
            uvw[1]),
        mix(
            mix(Values[1][0][0], Values[1][0][1], uvw[2]),
            mix(Values[1][1][0], Values[1][1][1], uvw[2]),
            uvw[1]),
        uvw[0]);
}

float TrilinearSampling(float Step, VdbRay ray, pnanovdb_buf_t buf, pnanovdb_uint32_t grid_type, pnanovdb_readaccessor_t acc)
{
    pnanovdb_vec3_t pos = pnanovdb_hdda_ray_start(ray.Origin, Step, ray.Direction);
    return TrilinearSampling(pos, buf, grid_type, acc);
}

bool CheckBounds(inout VdbRay Ray, pnanovdb_vec3_t bbox_min, pnanovdb_vec3_t bbox_max)
{
    return pnanovdb_hdda_ray_clip(bbox_min, bbox_max, Ray.Origin, Ray.TMin, Ray.Direction, Ray.TMax);
}

vec3 WorldToIndexDirection(vec3 WorldDirection, mat4 WorldToLocal, pnanovdb_buf_t buf, pnanovdb_grid_handle_t grid)
{
    vec3 Dir = (WorldToLocal * vec4(WorldDirection, 0.0)).xyz;
    return normalize(pnanovdb_grid_world_to_index_dirf(buf, grid, Dir));
}

vec3 WorldToIndexPosition(vec3 WorldPos, mat4 WorldToLocal, pnanovdb_buf_t buf, pnanovdb_grid_handle_t grid)
{
    vec3 Pos = (WorldToLocal * vec4(WorldPos, 1.0)).xyz;
    return pnanovdb_grid_world_to_indexf(buf, grid, Pos);
}

vec3 IndexToWorldDirection(vec3 IndexDirection, mat4 LocalToWorld, pnanovdb_buf_t buf, pnanovdb_grid_handle_t grid)
{
    vec3 LocalDir = pnanovdb_grid_index_to_world_dirf(buf, grid, IndexDirection);
    vec3 WorldDir = (LocalToWorld * vec4(LocalDir, 0.0)).xyz;
    return normalize(WorldDir);
}

vec3 IndexToWorldPosition(vec3 IndexPos, mat4 LocalToWorld, pnanovdb_buf_t buf, pnanovdb_grid_handle_t grid)
{
    vec3 LocalPos = pnanovdb_grid_index_to_worldf(buf, grid, IndexPos);
    return (LocalToWorld * vec4(LocalPos, 1.0)).xyz;
}

float IndexToWorldDistance(vec3 IndexVec, mat4 LocalToWorld, pnanovdb_buf_t buf, pnanovdb_grid_handle_t grid)
{
    vec3 LocalVec = pnanovdb_grid_index_to_world_dirf(buf, grid, IndexVec);
    vec3 WorldVec = (LocalToWorld * vec4(LocalVec, 0.0)).xyz;
    return length(WorldVec);
}

//Segment getRayFromPixelCoord(uint2 ScreenPosition, uint2 ScreenDimensions, float2 Jitter, float DeviceZ)
//{
//    float2 ScreenPositionNorm = (ScreenPosition + 0.5f + Jitter) / float2(ScreenDimensions);
//    float2 ClipPosition = (ScreenPositionNorm - View.ScreenPositionScaleBias.wz) / View.ScreenPositionScaleBias.xy;
//
//    DeviceZ = max(DeviceZ, 0.000000000000001); // either no Z value, or too far to even consider
//    float4 Near = mul(float4(ClipPosition, 1, 1), View.ClipToTranslatedWorld); // near (world space)
//    float4 Far = mul(float4(ClipPosition, DeviceZ, 1), View.ClipToTranslatedWorld); // scene gbuffer (world space)
//
//    Segment Seg;
//    Seg.Start = Near.xyz / Near.w; // Translated World
//    Seg.Start -= LWCHackToFloat(PrimaryView.PreViewTranslation); // World
//    Seg.End = Far.xyz / Far.w; // Translated World
//    Seg.End -= LWCHackToFloat(PrimaryView.PreViewTranslation); // World
//    return Seg;
//}

//VdbRay PrepareRayFromPixel(pnanovdb_buf_t grid_buf, pnanovdb_grid_handle_t grid, uint2 ScreenPosition, uint2 ScreenDimension, float2 Jitter, float DeviceZ, float4x4 WorldToLocal)
//{
//    // World space
//    Segment Seg = getRayFromPixelCoord(ScreenPosition.xy, ScreenDimension.xy, Jitter, DeviceZ);
//
//    // Index space
//    float3 Origin = WorldToIndexPosition(Seg.Start, WorldToLocal, grid_buf, grid);
//    float3 End = WorldToIndexPosition(Seg.End, WorldToLocal, grid_buf, grid);
//
//    float Dist = length(End - Origin);
//
//    VdbRay Ray;
//    Ray.Origin = Origin;
//    Ray.Direction = (End - Origin) / Dist;
//    Ray.TMin = 0.0001f;
//    Ray.TMax = DeviceZ == 0.0 ? POSITIVE_INFINITY : Dist;
//
//    return Ray;
//}

//-----------------------------------------------------------------------------------------------------------
// Level Set specific
//-----------------------------------------------------------------------------------------------------------

struct ZeroCrossingHit
{
    float t_hit;
    float v0;
    pnanovdb_coord_t ijk_hit;
};

pnanovdb_vec3_t ZeroCrossingNormal(pnanovdb_uint32_t grid_type, pnanovdb_buf_t grid_buf, pnanovdb_readaccessor_t acc, in ZeroCrossingHit ZCH)
{
    pnanovdb_coord_t ijk = ZCH.ijk_hit;
    pnanovdb_vec3_t iNormal = -ZCH.v0.xxx;

    ijk.x += 1;
    iNormal.x += ReadValue(ijk, grid_buf, grid_type, acc);

    ijk.x -= 1;
    ijk.y += 1;
    iNormal.y += ReadValue(ijk, grid_buf, grid_type, acc);

    ijk.y -= 1;
    ijk.z += 1;
    iNormal.z += ReadValue(ijk, grid_buf, grid_type, acc);

    return normalize(iNormal);
}

pnanovdb_bool_t GetNextIntersection(
    VdbSampler LsSampler,
in out VdbRay iRay,
in out ZeroCrossingHit HitResults)
{
    return pnanovdb_hdda_zero_crossing_improved(LsSampler.GridType, LsSampler.GridBuffer, LsSampler.Accessor, iRay.Origin, iRay.TMin, iRay.Direction, iRay.TMax, HitResults.t_hit, HitResults.v0, HitResults.ijk_hit);
}

//-----------------------------------------------------------------------------------------------------------
// Fog Volume specific
//-----------------------------------------------------------------------------------------------------------

//float DeltaTracking(in VdbRay Ray, pnanovdb_buf_t buf, pnanovdb_uint32_t grid_type, pnanovdb_readaccessor_t acc, HeterogenousMedium medium, inout RandomSequence RandSequence)
//{
//    float densityMaxInv = 1.0f / medium.densityMax;
//    float t = Ray.TMin;
//    pnanovdb_vec3_t pos;
//
//    do {
//        t += -log(RandomSequence_GenerateSample1D(RandSequence)) * densityMaxInv;
//        pos = pnanovdb_hdda_ray_start(Ray.Origin, t, Ray.Direction);
//    } while (t < Ray.TMax && ReadValue(pos, buf, grid_type, acc) * medium.densityScale * densityMaxInv < RandomSequence_GenerateSample1D(RandSequence));
//
//    return t;
//}

pnanovdb_vec3_t sampleHG(float g, float e1, float e2)
{
    // phase function.
    if (g == 0) {
        // isotropic
        const float phi = (2.0f * PI) * e1;
        const float cosTheta = 1.0f - 2.0f * e2;
        const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
        return pnanovdb_vec3_t(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    }
    else {
        const float phi = (2.0f * PI) * e2;
        const float s = 2.0f * e1 - 1.0f;
        const float denom = max(0.001f, (1.0f + g * s));
        const float f = (1.0f - g * g) / denom;
        const float cosTheta = 0.5f * (1.0f / g) * (1.0f + g * g - f * f);
        const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
        return pnanovdb_vec3_t(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    }
}

float PhaseHG(float CosTheta, float g)
{
    float denom = 1.0 + g * g - 2.0 * g * CosTheta;
    return 1.0 / (4.0 * PI) * (1.0 - g * g) / (denom * sqrt(denom));
}

// From NanoVDB samples
//float GetTransmittance(
//    pnanovdb_vec3_t bbox_min,
//    pnanovdb_vec3_t bbox_max,
//    VdbRay ray,
//    pnanovdb_buf_t buf,
//    pnanovdb_uint32_t grid_type,
//    pnanovdb_readaccessor_t acc,
//    HeterogenousMedium medium,
//    float StepMultiplier,
//in out RandomSequence RandSequence)
//{
//    pnanovdb_bool_t hit = pnanovdb_hdda_ray_clip(bbox_min, bbox_max, ray.Origin, ray.TMin, ray.Direction, ray.TMax);
//    if (!hit)
//    return 1.0f;
//
//    float densityMaxInv = 1.0f / medium.densityMax;
//    float densityMaxInvMultStep = densityMaxInv * StepMultiplier;
//    float transmittance = 1.f;
//    float t = ray.TMin;
//    while (true)
//    {
//        t += densityMaxInvMultStep * (RandomSequence_GenerateSample1D(RandSequence) + 0.5);
//        if (t >= ray.TMax)
//        break;
//
//        float density = ReadValue(t, ray, buf, grid_type, acc) * medium.densityScale;
//
//        transmittance *= 1.0f - density * densityMaxInv;
//        if (transmittance < 0.1f)
//        return 0.f;
//    }
//    return transmittance;
//}

// Cf FLinearColor::MakeFromColorTemperature
vec3 ColorTemperatureToRGB(float Temp)
{
    if (Temp < 1000.0f)
    return vec3(0.0f, 0.0f, 0.0f);

    Temp = clamp(Temp, 1000.0f, 15000.0f);

    // Approximate Planckian locus in CIE 1960 UCS
    float u = (0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp * Temp) / (1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp * Temp);
    float v = (0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp * Temp) / (1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp * Temp);

    float x = 3.0f * u / (2.0f * u - 8.0f * v + 4.0f);
    float y = 2.0f * v / (2.0f * u - 8.0f * v + 4.0f);
    float z = 1.0f - x - y;

    float Y = 1.0f;
    float X = Y / y * x;
    float Z = Y / y * z;

    // XYZ to RGB with BT.709 primaries
    float R = 3.2404542f * X + -1.5371385f * Y + -0.4985314f * Z;
    float G = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
    float B = 0.0556434f * X + -0.2040259f * Y + 1.0572252f * Z;

    return vec3(R, G, B);
}

float Average(vec3 Value)
{
    return dot(Value, vec3(1.0 / 3.0));
}


#endif // NANOVDB_VDB_COMMON_H_HAS_BEEN_INCLUDED