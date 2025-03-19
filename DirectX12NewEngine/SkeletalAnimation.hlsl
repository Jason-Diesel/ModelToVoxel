struct VertexInput
{
    float3 position : Position;
    float2 uv : UV;
    float3 normal : Normal;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 BoneIds : BoneIds;
    float4 BoneWeights : BoneWeights;
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

cbuffer SkeletalAnimation : register(b4)
{
    row_major matrix SkeletalMatrix[65];
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    const int NrOfBonesVertexIsAttachedTo = 4;
    float4x4 mvp = mul(mul(transform, view), projection);
    output.uv = input.uv;
    
    float4 totalPosition = float4(0,0,0,0);
    for (int i = 0; i < NrOfBonesVertexIsAttachedTo; i++)
    {
        if (input.BoneIds[i] < -0.5 || input.BoneIds[i] >= 65)
        {
            break;
        }
        float4 localPosition = mul(SkeletalMatrix[input.BoneIds[i]], float4(input.position, 1.0f));
        totalPosition += localPosition * input.BoneWeights[i];
    }

    //output.position = mul(float4(input.position.xyz, 1.0), mvp);
    output.position = mul(totalPosition, mvp);
    output.fragpos = mul(float4(input.position, 1.0f), transform);

    output.TBN = float3x3(
        normalize((mul(float4(input.tangent, 0), transform)).xyz),
        normalize((mul(float4(input.bitangent, 0), transform)).xyz),
        normalize((mul(float4(input.normal, 0), transform)).xyz)
    );

    return output;
}