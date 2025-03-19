#pragma once
#include "CommonHeaders.h"

struct OFTVertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
};

struct Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 bitangent;
};

struct SkeletalVertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 bitangent;

	DirectX::XMINT4 boneIds = DirectX::XMINT4(-1,-1,-1,-1);//What bones the vertex is stuck to
	DirectX::XMFLOAT4 boneWeight = DirectX::XMFLOAT4(0,0,0,0);//How strong the bones are stuck to the bone
};