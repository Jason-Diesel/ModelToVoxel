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
    int4 materialIndex; //Diffuse, LOD
}

Texture3D<uint> bindless_Voxels : register(t0, space0);

SamplerState samp : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
    if (materialIndex.x >= MAXNROFMATERIALS || materialIndex.x < 0)
    {
        return float4(1, 0, 0, 1);
    }
    
    int4 location = int4(input.localPosition.xyz, 0);
    
    uint voxelColor = bindless_Voxels.Load(location);
    
    if (voxelColor == 0)
    {
        discard;
    }
    
    const float3 ka = float3(0.3, 0.3, 0.3);
    const float Shininess = 32.f;
    const float specularStrength = 0.5f;
    const float3 viewDir = normalize(input.fragpos.xyz - cameraPos.xyz);
    
    float3 color = float3(
            (voxelColor & 0xFF) / 255.0f,
            ((voxelColor >> 8) & 0xFF) / 255.0f,
            ((voxelColor >> 16) & 0xFF) / 255.0f
        );
    
    float3 resultColor = float3(0, 0, 0);

    return float4(color, 1.0f);

}