#pragma once
#include "Object.h"

struct ChunkConstantBuffer {
	DirectX::XMINT2 bindlessTextureIndex;
};

class Chunk : public Object{
public:
	Chunk(Graphics* gfx);
	void setConstantBuffers(Graphics* gfx);
	void updateConstantBuffers();
	void setLOD(const uint32_t lod);
	uint32_t getLod() const;
	~Chunk();
public:
	ChunkConstantBuffer cbData;
	ConstantBuffer materialIndexBuffer;
};