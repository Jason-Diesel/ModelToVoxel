struct VoxelData
{
    uint colorValue;
};

/*
    0 = Voxels (uint32_t) (UAV)
    1 = NrOfIndecies (uint32_t), BoundingBoxes (2 float3), Material Index (uint32_t), VoxelSize (float)
    2 = Vertecies (float3 + float2)
    3 = Indecies (uint32_t)
    4... = Materials (Texture)
*/
RWStructuredBuffer<VoxelData> bindless_Voxels[1] : register(u0, space0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x;
    bindless_Voxels[0][index].colorValue = 1234567;
}