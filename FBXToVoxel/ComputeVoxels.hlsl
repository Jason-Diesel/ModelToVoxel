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
    float4 voxelSize;//W is nr of textures
};

#define FLT_MAX 3.402823466e+38F

RWStructuredBuffer<voxelData> VoxelGrid : register(u0);

//The Mesh
RWStructuredBuffer<Vertex> Vertecies : register(u1);
RWStructuredBuffer<uint> Indecies : register(u2);
RWStructuredBuffer<int4> WhatTexture : register(u3); //Mesh first indecies ID, mesh last indecies ID, mesh Texture CPU, MeshTexture GPU, and again 

Texture2D<float4> MeshTexture[200] : register(t0); //Make this into a UAV?

SamplerState samp : register(s0);

uint getIndex(const uint x, const uint y, const uint z, const uint width, const uint height)
{
    return x + (y * width) + (z * width * height);
}

int getTextureID(uint indeciesID)
{
    //uint i = 0;
    //while(i < voxelSize.w )
    //{
    //    if (indeciesID >= WhatTexture[i].x && indeciesID < WhatTexture[i].y)
    //    {
    //        return WhatTexture[i].w;
    //    }
    //    i++;
    //}
    //return 0;
    
    uint left = 0;
    uint right = voxelSize.w - 1;

    while (left <= right)
    {
        uint mid = (left + right) / 2;
        int4 range = WhatTexture[mid];
        
        if (indeciesID < range.x)
        {
            right = mid - 1;
        }
        else if (indeciesID >= range.y)
        {
            left = mid + 1;
        }
        else
        {
            return range.w;
        }
    }
    return 0;
}

void lineToLine(
    uint3 startVoxel,
    uint3 endVoxel,
    float2 startUV,
    float2 endUV,
    const uint3 sizes,
    const uint textureId
)
{
    uint3 traverseVoxel = startVoxel;
    
    const int3 distanceV = int3(
        endVoxel.x - startVoxel.x,
        endVoxel.y - startVoxel.y,
        endVoxel.z - startVoxel.z
    );
    
    const int3 stepDirection = int3(
        (distanceV.x > 0) ? 1 : distanceV.x < 0 ? -1 : 0,
        (distanceV.y > 0) ? 1 : distanceV.y < 0 ? -1 : 0,
        (distanceV.z > 0) ? 1 : distanceV.z < 0 ? -1 : 0
    );
    
    const float l = distance(distanceV, float3(0, 0, 0));

    const float3 lineDirection = float3(
        distanceV.x / l,
        distanceV.y / l,
        distanceV.z / l
    );
    
    const float3 stepLength = float3(
        stepDirection.x != 0 ? abs(1.0f / lineDirection.x) : FLT_MAX,
        stepDirection.y != 0 ? abs(1.0f / lineDirection.y) : FLT_MAX,
        stepDirection.z != 0 ? abs(1.0f / lineDirection.z) : FLT_MAX
    );
    
    float3 tMax = float3(
        0,
        0,
        0
    );
    
    const float totalDist = distance(startVoxel, endVoxel);
    
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
        const float procent = distance(startVoxel, traverseVoxel) / totalDist;
        const float2 uv = lerp(startUV, endUV, saturate(procent));
        
        const float4 color = MeshTexture[textureId].SampleLevel(samp, uv, 0);
        
        if(color.w != 0)
        {
            const int index = getIndex(traverseVoxel.x, traverseVoxel.y, traverseVoxel.z >= sizes.z ? sizes.z - 1 : traverseVoxel.z, sizes.x, sizes.y);
            const uint3 currentColor = VoxelGrid[index].color;
            if (currentColor.x == 0 && currentColor.y == 0 && currentColor.z == 0)
            {
                VoxelGrid[index].color = uint3(color.x * 255, color.y * 255, color.z * 255);
                if (color.x + color.y + color.z == 0)
                {
                    VoxelGrid[index].color.x = 1;
                }
            }
        }
    }
}

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{    
    //For every triangle
    uint index = DTid.x * 3;
    
    if(index > sizes.w)
    {
        return;
    }
    
    int textureID = getTextureID(index);
    
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
    [unroll]
    for (uint j = 0; j < 3; j++)
    {
        const float3 positionInGrid = (triangleVertecies[j].position - minSizes.xyz) / voxelSize.x;
        voxelPosition[j] = int3(positionInGrid.x, positionInGrid.y, positionInGrid.z);
    }
    
    //From a -> b and everytime we get a block, C to a <-> b
    const uint3 startVoxel = voxelPosition[0];
    uint3 traverseVoxel = startVoxel;
    const uint3 endVoxel = voxelPosition[1];
    const uint3 linerVoxel = voxelPosition[2];
    
    const int3 distanceV = int3(
        endVoxel.x - startVoxel.x,
        endVoxel.y - startVoxel.y,
        endVoxel.z - startVoxel.z
    );
    
    const int3 stepDirection = int3(
        distanceV.x > 0 ? 1 : distanceV.x < 0 ? -1 : 0,
        distanceV.y > 0 ? 1 : distanceV.y < 0 ? -1 : 0,
        distanceV.z > 0 ? 1 : distanceV.z < 0 ? -1 : 0
    );
    
    const float l = distance(float3(distanceV.x, distanceV.y, distanceV.z), float3(0,0,0));
    
    const float3 lineDirection = float3(
                distanceV.x / l,
                distanceV.y / l,
                distanceV.z / l
            );
    
    const float3 stepLength = float3(
        stepDirection.x != 0 ? abs(1.0f / lineDirection.x) : FLT_MAX, //Cannot find float max value, so just a big number
        stepDirection.y != 0 ? abs(1.0f / lineDirection.y) : FLT_MAX, //Cannot find float max value, so just a big number
        stepDirection.z != 0 ? abs(1.0f / lineDirection.z) : FLT_MAX //Cannot find float max value, so just a big number
    );
    
    float3 tMax = float3(
        0,
        0,
        0
    );
    
    const float totalDistance = distance(startVoxel, endVoxel);
    
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
        
        //Unsure if we gonna add it here also?
        float procent = distance(traverseVoxel, startVoxel) / totalDistance;
        float2 uv = lerp(triangleVertecies[0].uv, triangleVertecies[1].uv, saturate(procent));
        
        //Do line to line with triangleVertecies[2] and traverselVertex
        lineToLine(linerVoxel, traverseVoxel, Vertecies[triangleIndecies[2]].uv, uv, sizes.xyz, textureID);
    }
}