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
    float3x3 TBN : TBN;
    float2 uv : UV;
    float4 fragpos : FRAG_POS;
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

    output.uv = input.uv;
    output.position = float4(input.position.xyz, 1.0);
    output.fragpos = float4(input.position, 1.0f);

    output.TBN = float3x3(
        normalize(input.tangent.xyz),
        normalize(input.bitangent.xyz),
        normalize(input.normal.xyz)
    );

    return output;
}
