#pragma once
#include "CommonHeaders.h"

struct Vertecies {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
};

struct Voxel {
    uint16_t rgb[3] = { 0, 0, 0 };
};
struct VoxelGPU {
    uint32_t rgb[3] = { 0, 0, 0 };
};

struct VoxelVertecies {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
};