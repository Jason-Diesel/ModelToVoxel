struct voxelData
{
    uint3 color;
};
struct Vertex
{
    float3 position;
    float2 uv;
};

cbuffer Data : register(b0)
{
    uint4 sizes;//W is equal to nrOfIndecies
    float4 minSizes;
    float4 voxelSize;
};

RWStructuredBuffer<voxelData> VoxelGrid : register(u0);

//The Mesh
RWStructuredBuffer<Vertex> Vertecies : register(u1);
RWStructuredBuffer<uint> Indecies : register(u2);
Texture2D<float4> MeshTexture : register(t0);//Make this into a UAV?

SamplerState samp : register(s0);

uint getIndex(const uint x, const uint y, const uint z, const uint width, const uint height)
{
    return x + (y * width) + (z * width * height);
}

void lineToLine(
    uint3 startVoxel,
    uint3 endVoxel,
    float2 startUV,
    float2 endUV,
    uint3 sizes
)
{
    uint3 traverseVoxel = startVoxel;
    
    int3 distanceV = int3(
        endVoxel.x - startVoxel.x,
        endVoxel.y - startVoxel.y,
        endVoxel.z - startVoxel.z
    );
    
    int3 stepDirection = int3(
        (distanceV.x > 0) ? 1 : distanceV.x < 0 ? -1 : 0,
        (distanceV.y > 0) ? 1 : distanceV.y < 0 ? -1 : 0,
        (distanceV.z > 0) ? 1 : distanceV.z < 0 ? -1 : 0
    );
    
    float l = distance(distanceV, float3(0, 0, 0));

    float3 lineDirection = float3(
        distanceV.x / l,
        distanceV.y / l,
        distanceV.z / l
    );
    
    float3 stepLength = float3(
        stepDirection.x != 0 ? abs(1.0f / lineDirection.x) : 9999999,
        stepDirection.y != 0 ? abs(1.0f / lineDirection.y) : 9999999,
        stepDirection.z != 0 ? abs(1.0f / lineDirection.z) : 9999999
    );
    
    float3 tMax = float3(
        0,
        0,
        0
    );
    
    float totalDist = distance(startVoxel, endVoxel);
    
    while (!(traverseVoxel.x == endVoxel.x && traverseVoxel.y == endVoxel.y && traverseVoxel.z == endVoxel.z))
    {
        if (tMax.x < tMax.y && tMax.x < tMax.z)
        {
            traverseVoxel.x += stepDirection.x;
            tMax.x += stepLength.x;
        }
        else if (tMax.y < tMax.z)
        {
            traverseVoxel.y += stepDirection.y;
            tMax.y += stepLength.y;
        }
        else
        {
            traverseVoxel.z += stepDirection.z;
            tMax.z += stepLength.z;
        }
        float procent = distance(startVoxel, traverseVoxel) / totalDist;
        float2 uv = lerp(startUV, endUV, saturate(procent));
        
        float4 color = MeshTexture.SampleLevel(samp, uv, 0);
        
        if(color.w != 0)
        {
            int index = getIndex(traverseVoxel.x, traverseVoxel.y, traverseVoxel.z >= sizes.z ? sizes.z - 1 : traverseVoxel.z, sizes.x, sizes.y);
            VoxelGrid[index].color = uint3(color.x * 255, color.y * 255, color.z * 255);
            if (color.x + color.y + color.z == 0)
            {
                VoxelGrid[index].color.x = 1;

            }
        }
    }
}

[numthreads(128, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{    
    //For every triangle
    uint index = DTid.x * 3;
    
    if(index > sizes.w)
    {
        return;
    }
    uint triangleIndecies[3] =
    {
        Indecies[(index + 0)],
        Indecies[(index + 1)],
        Indecies[(index + 2)]
    };
    
    Vertex triangleVertecies[3] =
    {
        Vertecies[triangleIndecies[0]],
        Vertecies[triangleIndecies[1]],
        Vertecies[triangleIndecies[2]]
    };
    
    int3 voxelPosition[3];
    for (uint j = 0; j < 3; j++)
    {
        float3 positionInGrid = (triangleVertecies[j].position - minSizes.xyz) / voxelSize.x;
        voxelPosition[j] = int3(positionInGrid.x, positionInGrid.y, positionInGrid.z);
        
        int BlockIndex = getIndex(voxelPosition[j].x, voxelPosition[j].y, voxelPosition[j].z >= sizes.z ? sizes.z - 1 : voxelPosition[j].z, sizes.x, sizes.y);
        float4 color = MeshTexture.SampleLevel(samp, triangleVertecies[j].uv, 0);
    
        VoxelGrid[BlockIndex].color = uint3(color.x * 255.f, color.y * 255.f, color.z * 255.f);
    }
    
    //From a -> b and everytime we get a block, C to a <-> b
    const uint3 startVoxel = voxelPosition[0];
    uint3 traverseVoxel = startVoxel;
    const uint3 endVoxel = voxelPosition[1];
    const uint3 linerVoxel = voxelPosition[2];
    
    int3 distanceV = int3(
        endVoxel.x - startVoxel.x,
        endVoxel.y - startVoxel.y,
        endVoxel.z - startVoxel.z
    );
    
    int3 stepDirection = int3(
        distanceV.x > 0 ? 1 : distanceV.x < 0 ? -1 : 0,
        distanceV.y > 0 ? 1 : distanceV.y < 0 ? -1 : 0,
        distanceV.z > 0 ? 1 : distanceV.z < 0 ? -1 : 0
    );
    
    float l = distance(float3(distanceV.x, distanceV.y, distanceV.z), float3(0,0,0));
    
    float3 lineDirection = float3(
                distanceV.x / l,
                distanceV.y / l,
                distanceV.z / l
            );
    
    float3 stepLength = float3(
        stepDirection.x != 0 ? abs(1.0f / lineDirection.x) : 9999999,//Cannot find float max value, so just a big number
        stepDirection.y != 0 ? abs(1.0f / lineDirection.y) : 9999999,//Cannot find float max value, so just a big number
        stepDirection.z != 0 ? abs(1.0f / lineDirection.z) : 9999999 //Cannot find float max value, so just a big number
    );
    
    float3 tMax = float3(
        0,
        0,
        0
    );
    
    float totalDistance = distance(startVoxel, endVoxel);
    
    while (!(traverseVoxel.x == voxelPosition[1].x && traverseVoxel.y == voxelPosition[1].y && traverseVoxel.z == voxelPosition[1].z))
    {
        if (tMax.x < tMax.y && tMax.x < tMax.z)
        {
            traverseVoxel.x += stepDirection.x;
            tMax.x += stepLength.x;
        }
        else if (tMax.y < tMax.z)
        {
            traverseVoxel.y += stepDirection.y;
            tMax.y += stepLength.y;
        }
        else
        {
            traverseVoxel.z += stepDirection.z;
            tMax.z += stepLength.z;
        }
        
        Vertex startVertex = triangleVertecies[0];
        Vertex endVertex = triangleVertecies[1];
        
        //Unsure if we gonna add it here also?
        float procent = distance(traverseVoxel, startVoxel) / totalDistance;
        float2 uv = lerp(startVertex.uv, endVertex.uv, saturate(procent));
        int BlockIndex = getIndex(traverseVoxel.x, traverseVoxel.y, traverseVoxel.z >= sizes.z ? sizes.z - 1 : traverseVoxel.z, sizes.x, sizes.y);
        
        //Do line to line with triangleVertecies[2] and traverselVertex
        lineToLine(linerVoxel, traverseVoxel, Vertecies[triangleIndecies[2]].uv, uv, sizes.xyz);
    }
}