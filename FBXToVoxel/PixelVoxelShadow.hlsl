#include "../DirectX12NewEngine/DefaultShader/Defines.hlsli"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 localPosition : LocalPos;
    float3 normal : Normal;
    float4 fragpos : FragPos;
};

cbuffer MVP : register(b0)
{
    row_major matrix view;
    row_major matrix projection;
    float4 cameraPos;
};

cbuffer MaterialBuffer : register(b2)
{
    int4 materialIndex;
}

//COLORS are stored in one uint32_t but are 3 different uint8_t values
Texture3D<uint> bindless_Voxels : register(t0, space0);

Texture3D<uint> GetVoxles(uint textureIndex)
{
    return bindless_Voxels;
}

SamplerState samp : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    if (materialIndex.x >= MAXNROFMATERIALS || materialIndex.x < 0)
    {
        return float4(1, 0, 0, 1);
    }
    
    int4 location = int4(input.localPosition.xyz, 0);
    //if (location.x > 128 || location.y > 128 || location.z > 128 ||
    //    location.x < 0 || location.y < 0 || location.z < 0)
    //{
    //    discard;
    //}
    
    uint voxelColor = GetVoxles(materialIndex.x).Load(location);
    
    if (voxelColor == 0)
    {
        discard;
    }
    return float4(1, 0, 0, 1);
}