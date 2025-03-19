struct PixelShaderInput
{
    float4 position : SV_POSITION;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return float4(1, 0, 0, 1);
}