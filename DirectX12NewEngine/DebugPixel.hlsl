struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3x3 TBN : TBN;
    float2 uv : UV;
    float4 fragpos : FRAG_POS;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}