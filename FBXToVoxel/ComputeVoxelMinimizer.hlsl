
RWTexture3D<uint> BigVoxels : register(u0);
RWTexture3D<uint> SmallerVoxels : register(u1);

[numthreads(8, 8, 8)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint3 outputSize;
    SmallerVoxels.GetDimensions(outputSize.x, outputSize.y, outputSize.z);
    uint3 inputSize = outputSize * 2;
    uint3 inputCoord = DTid * 2;
    
    float4 color = (
        BigVoxels[inputCoord + uint3(0, 0, 0)] +
        BigVoxels[inputCoord + uint3(1, 0, 0)] +
        BigVoxels[inputCoord + uint3(0, 1, 0)] +
        BigVoxels[inputCoord + uint3(1, 1, 0)] +
        BigVoxels[inputCoord + uint3(0, 0, 1)] +
        BigVoxels[inputCoord + uint3(1, 0, 1)] +
        BigVoxels[inputCoord + uint3(0, 1, 1)] +
        BigVoxels[inputCoord + uint3(1, 1, 1)]
    ) / 8.0f;

    SmallerVoxels[DTid] = color;

}