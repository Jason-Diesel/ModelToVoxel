struct VertexInput
{
    float3 position : Position;
    float2 uv : UV;
    float3 normal : Normal;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
};

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

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    float4x4 mvp = mul(mul(transform, view), projection);
    output.position = mul(float4(input.position.xyz, 1.0), mvp);

    return output;
}
