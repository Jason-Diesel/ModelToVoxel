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
    
    float4x4 mvp = mul(mul(transform, view), projection);
    output.uv = input.uv;
    output.position = mul(float4(input.position.xyz, 1.0), mvp);
    output.fragpos = mul(float4(input.position, 1.0f), transform); //Multiply with transform
    
    output.TBN = float3x3(
        normalize((mul(float4(input.tangent, 0), transform)).xyz),
        normalize((mul(float4(input.bitangent, 0), transform)).xyz),
        normalize((mul(float4(input.normal, 0), transform)).xyz)
    );
    
    return output;
}
