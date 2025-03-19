#pragma once
#include "Material.h"
#include "Animation.h"

struct Mesh {
	virtual ~Mesh();
	Material* material = nullptr;//Material is deleted in resource manager
	uint32_t nrOfVertecies;
	uint32_t nrOfIndecies;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indeciesBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indeciesBuffer;
};