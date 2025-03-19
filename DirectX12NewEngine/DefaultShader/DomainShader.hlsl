#include "Defines.hlsli"

cbuffer MVP : register(b0)
{
    row_major matrix view;
    row_major matrix projection;
    float4 cameraPos;
};

cbuffer Transform : register(b1)
{
    row_major matrix transform;
};

struct HS_INPUT_FVS
{
    float4 position : SV_POSITION;
    float3x3 TBN : TBN;
    float2 uv : UV;
    float4 fragpos : FRAG_POS;
};

struct HS_OUTPUT
{
    float4 position : SV_POSITION;
    float3x3 TBN : TBN;
    float2 uv : UV;
    float4 fragpos : FRAG_POS;
};

// Output patch constant data.
struct HS_CONSTANT_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

float3 calcPlanePointPos(float3 normal, float3 planepos, float3 origin)
{
    normal = normalize(normal);
    float d = dot(normal, planepos);
    float t = (d - dot(normal, origin)) / dot(normal, normal);
    float3 thePos = origin + (normal * t);
    return thePos;
};

cbuffer MaterialBuffer : register(b2)
{
    float4 ka;
    float4 kd;
    float4 ks;
    float4 ns;
    int materialFlag;
    int4 materialIndex;//Diffuse, Normal, Height, LightTexture
}

Texture2D<float4> bindless_textures[MAXNROFMATERIALS] : register(t0, space0);

SamplerState samp : register(s0);

#define NUM_CONTROL_POINTS 3

[domain("tri")]
HS_INPUT_FVS main(
    HS_CONSTANT_OUTPUT input,
    float3 domain : SV_DomainLocation,
    const OutputPatch<HS_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    HS_INPUT_FVS Output;
    float4x4 MVP = mul(mul(transform, view), projection);

    Output.uv = patch[0].uv.xy * domain.x + patch[1].uv.xy * domain.y + patch[2].uv.xy * domain.z;

    Output.TBN[2] = patch[0].TBN[2] * domain.x + patch[1].TBN[2] * domain.y + patch[2].TBN[2] * domain.z;
    Output.TBN[0] = patch[0].TBN[0] * domain.x + patch[1].TBN[0] * domain.y + patch[2].TBN[0] * domain.z;
    Output.TBN[1] = patch[0].TBN[1] * domain.x + patch[1].TBN[1] * domain.y + patch[2].TBN[1] * domain.z;

    Output.position = float4(patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z);
    Output.fragpos = float4(patch[0].fragpos * domain.x + patch[1].fragpos * domain.y + patch[2].fragpos * domain.z);

    Output.TBN[0] = normalize((mul(float4(Output.TBN[0], 0.0f), transform)).xyz); //
    Output.TBN[1] = normalize((mul(float4(Output.TBN[1], 0.0f), transform)).xyz); //
    Output.TBN[2] = normalize((mul(float4(Output.TBN[2], 0.0f), transform)).xyz);  //

    //PHONG TESS
    if (!(materialFlag & MaterialType_Height))
    {
        float3 P = float3(patch[0].position.xyz * domain.x + patch[1].position.xyz * domain.y + patch[2].position.xyz * domain.z);
        float alpha = 0.75f;

        float3 q0 = calcPlanePointPos(patch[0].TBN[2], patch[0].position.xyz, patch[0].position.xyz * domain.x + patch[1].position.xyz * domain.y + patch[2].position.xyz * domain.z);
        float3 q1 = calcPlanePointPos(patch[1].TBN[2], patch[1].position.xyz, patch[0].position.xyz * domain.x + patch[1].position.xyz * domain.y + patch[2].position.xyz * domain.z);
        float3 q2 = calcPlanePointPos(patch[2].TBN[2], patch[2].position.xyz, patch[0].position.xyz * domain.x + patch[1].position.xyz * domain.y + patch[2].position.xyz * domain.z);
        float3x3 PIijk = float3x3(
            q0, q1, q2
        );

        Output.position = float4((1 - alpha) * P + mul((alpha * domain.xyz), PIijk), 1);

        Output.fragpos = mul(Output.position, transform);

        Output.position = mul(Output.position, MVP);
    }
    else
    {//HEIGHTMAP
        const float heightMultiplier = 0.4;
        float heightSample = bindless_textures[materialIndex.y].SampleLevel(samp, Output.uv, 0).x;
        float3 normal = Output.TBN[2];
        Output.position += float4(normal * (heightSample * heightMultiplier), 0);
        Output.position = mul(Output.position, MVP);
    }

    return Output;
}

/*

struct DS_OUTPUT
{
    float4 position : SV_POSITION;
    float3x3 TBN : TBN;
    float2 uv : UV;
    float4 fragpos : FRAG_POS;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float4 position : SV_POSITION;
    float3x3 TBN : TBN;
    float2 uv : UV;
    float4 fragpos : FRAG_POS;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
    HS_CONSTANT_DATA_OUTPUT input,
    float3 domain : SV_DomainLocation,
    const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT Output;
    
    Output.position =
    patch[0].position * domain.x +
    patch[1].position * domain.y +
    patch[2].position * domain.z;

    Output.TBN[0] =
    patch[0].TBN[0] * domain.x +
    patch[1].TBN[0] * domain.y +
    patch[2].TBN[0] * domain.z;
    Output.TBN[1] =
    patch[0].TBN[1] * domain.x +
    patch[1].TBN[1] * domain.y +
    patch[2].TBN[1] * domain.z;
    Output.TBN[2] =
    patch[0].TBN[2] * domain.x +
    patch[1].TBN[2] * domain.y +
    patch[2].TBN[2] * domain.z;

    Output.uv =
    patch[0].uv * domain.x +
    patch[1].uv * domain.y +
    patch[2].uv * domain.z;

    Output.fragpos =
    patch[0].fragpos * domain.x +
    patch[1].fragpos * domain.y +
    patch[2].fragpos * domain.z;

    return Output;
}
*/