#pragma once
#include "Object.h"

struct ChunkConstantBuffer {
	uint32_t bindlessTextureIndex;
};

class Chunk : public Object{
public:
	Chunk(Graphics* gfx);
	void setConstantBuffers(Graphics* gfx);
	void updateConstantBuffers();
	~Chunk();
public:
	ChunkConstantBuffer cbData;
	ConstantBuffer materialIndexBuffer;
};