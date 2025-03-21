struct VertexInput
{
    float3 position : Position;
    float3 normal : Normal;
};

struct VertexOutput
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
    float4 cameraPosition;
};

cbuffer Transform : register(b1)
{
    row_major matrix transform;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;

    const float bias = 0.3f;
    
    float4x4 mvp = mul(mul(transform, view), projection);
    //Calculate the normal
    output.normal = normalize(mul(float4(input.normal, 0), transform).xyz);
    output.fragpos = mul(float4(input.position, 1), transform);
    float3 viewDir = normalize(output.fragpos.xyz - cameraPosition.xyz);
    if (dot(viewDir, output.normal) > 0)
    {
        //Flip the normal
        output.normal = -output.normal;
    }
    
    //calculate the rest
    output.position.xyz = input.position.xyz + (output.normal * 0.01);
    output.position = mul(float4(output.position.xyz, 1.0), mvp);
    
    output.localPosition.xyz = input.position - (output.normal * bias);
    
    return output;
}