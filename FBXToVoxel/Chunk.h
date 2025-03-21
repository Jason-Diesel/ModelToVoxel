#pragma once
#include "Object.h"
#define NROFLOD 5

struct ChunkConstantBuffer {
	DirectX::XMINT2 bindlessTextureIndex;
};

class Chunk : public Object{
public:
	Chunk(Graphics* gfx);
	void setConstantBuffers(Graphics* gfx);
	void updateConstantBuffers();
	void setLOD(const uint32_t lod);
	void setTexturePointerForLod(const uint32_t& texturePtr, const uint32_t& lod);
	uint32_t getLod() const;
	uint32_t getTexturePointerFromLod(const uint32_t& lod);
	~Chunk();
public:
	uint32_t PtrToModelLOD[NROFLOD];
	uint32_t lod = 0;
	ChunkConstantBuffer cbData;
	ConstantBuffer materialIndexBuffer;
};