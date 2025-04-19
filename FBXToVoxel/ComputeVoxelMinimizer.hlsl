RWTexture3D<uint> BigVoxels : register(u0);
RWTexture3D<uint> SmallerVoxels : register(u1);

float4 getColor(uint voxelValue)
{
    if(voxelValue == 0)
    {
        return float4(0, 0, 0, 0);
    }
    return float4(
            (voxelValue & 0xFF),
            ((voxelValue >> 8) & 0xFF),
            ((voxelValue >> 16) & 0xFF),
            1
        );
}
     
uint getVoxelValue(float3 color)
{
    uint r = color.r;
    uint g = color.g;
    uint b = color.b;
    
    return (b << 16) | (g << 8) | r;
}

bool isInside(const uint3 image, const uint3 coords)
{
    return  coords.x >= 0 &&
            coords.y >= 0 &&
            coords.z >= 0 &&
            coords.x < image.x &&
            coords.y < image.y &&
            coords.z < image.z;

}

[numthreads(8, 8, 8)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint3 outputSize;
    SmallerVoxels.GetDimensions(outputSize.x, outputSize.y, outputSize.z);
    uint3 BiggerSize = outputSize * 2;
    uint3 inputCoord = DTid * 2;
    
    float4 color = (
        (isInside(BiggerSize, inputCoord + uint3(0, 0, 0)) ? getColor(BigVoxels[inputCoord + uint3(0, 0, 0)]) : float4(0, 0, 0, 0)) + 
        (isInside(BiggerSize, inputCoord + uint3(0, 0, 1)) ? getColor(BigVoxels[inputCoord + uint3(0, 0, 1)]) : float4(0, 0, 0, 0)) + 
        (isInside(BiggerSize, inputCoord + uint3(0, 1, 0)) ? getColor(BigVoxels[inputCoord + uint3(0, 1, 0)]) : float4(0, 0, 0, 0)) + 
        (isInside(BiggerSize, inputCoord + uint3(0, 1, 1)) ? getColor(BigVoxels[inputCoord + uint3(0, 1, 1)]) : float4(0, 0, 0, 0)) + 
        (isInside(BiggerSize, inputCoord + uint3(1, 0, 0)) ? getColor(BigVoxels[inputCoord + uint3(1, 0, 0)]) : float4(0, 0, 0, 0)) + 
        (isInside(BiggerSize, inputCoord + uint3(1, 0, 1)) ? getColor(BigVoxels[inputCoord + uint3(1, 0, 1)]) : float4(0, 0, 0, 0)) + 
        (isInside(BiggerSize, inputCoord + uint3(1, 1, 0)) ? getColor(BigVoxels[inputCoord + uint3(1, 1, 0)]) : float4(0, 0, 0, 0)) + 
        (isInside(BiggerSize, inputCoord + uint3(1, 1, 1)) ? getColor(BigVoxels[inputCoord + uint3(1, 1, 1)]) : float4(0, 0, 0, 0))
    );
    color /= color.w;
    color.r = color.r > 0 && color.r < 1 ? color.r + 1 : color.r;
    color.g = color.g > 0 && color.g < 1 ? color.g + 1 : color.g;
    color.b = color.b > 0 && color.b < 1 ? color.b + 1 : color.b;

    SmallerVoxels[DTid] = getVoxelValue(color.xyz);
}