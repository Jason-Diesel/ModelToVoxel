
// Input control point
struct HS_INPUT_FVS
{
    float4 position : SV_POSITION;
    float3x3 TBN : TBN;
    float2 uv : UV;
    float4 fragpos : FRAG_POS;
};

// Output patch constant data.
struct HS_CONSTANT_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
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


#define NUM_CONTROL_POINTS 3

float GetTessLevel(float Distance0, float Distance1)
{
    float AvgDistance = (Distance0 + Distance1) / 2.0;
    float MaxDistance = 150.f;
    float McDDistance = 50.f; //maccers is oppisete of max
    float MaxLod = 40;
    float mcDLod = 1;

    if (AvgDistance > MaxDistance)
    {
        AvgDistance = MaxDistance - 0.1;
    }
    AvgDistance -= MaxDistance;
    AvgDistance = -AvgDistance;
    
    float proc = AvgDistance / MaxDistance;
    return proc * MaxLod;
}

// Patch Constant Function
HS_CONSTANT_OUTPUT CalcHSPatchConstants(
	InputPatch<HS_INPUT_FVS, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_OUTPUT Output;
    float3 frag1 = mul(ip[0].position, transform).xyz;
    float3 frag2 = mul(ip[1].position, transform).xyz;
    float3 frag3 = mul(ip[2].position, transform).xyz;
    
    float dist0 = distance(frag1.xyz, cameraPos.xyz);
    float dist1 = distance(frag2.xyz, cameraPos.xyz);
    float dist2 = distance(frag3.xyz, cameraPos.xyz);
	
    Output.EdgeTessFactor[0] = GetTessLevel(dist1, dist2);
    Output.EdgeTessFactor[1] = GetTessLevel(dist2, dist0);
    Output.EdgeTessFactor[2] = GetTessLevel(dist0, dist1);
    
    Output.InsideTessFactor = Output.EdgeTessFactor[2];
    
    

    return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_INPUT_FVS main(
	InputPatch<HS_INPUT_FVS, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HS_INPUT_FVS Output;
	
    return ip[i];

}
