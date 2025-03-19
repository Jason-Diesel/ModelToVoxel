#pragma once
#include "CommonHeaders.h"

class TextureViewClass {
public:
	TextureViewClass();
	~TextureViewClass();
	Microsoft::WRL::ComPtr<ID3D12Resource> srvResource;
	D3D12_SRV_DIMENSION textureType;
	D3D12_UAV_DIMENSION UAVType;

	//DEBUG
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
};