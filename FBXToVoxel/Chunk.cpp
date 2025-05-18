#include "Chunk.h"

Chunk::Chunk(Graphics* gfx)
{
	materialIndexBuffer = CreateConstantBuffer<ChunkConstantBuffer>(gfx, ChunkConstantBuffer({0, 0}));
	transformBuffer = CreateConstantBuffer<DirectX::XMMATRIX>(gfx);
}

void Chunk::setConstantBuffers(Graphics* gfx)
{
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(2, materialIndexBuffer.constantBuffer->GetGPUVirtualAddress());
}

void Chunk::updateConstantBuffers()
{
	updateConstantBuffer(cbData, materialIndexBuffer);
}

void Chunk::setLOD(const uint32_t lod)
{
	this->lod = lod;
	cbData.bindlessTextureIndex.x = PtrToModelLOD[lod];
	cbData.bindlessTextureIndex.y = lod;
	updateConstantBuffers();
}

void Chunk::setTexturePointerForLod(const uint32_t& texturePtr, const uint32_t& lod)
{
	PtrToModelLOD[lod] = texturePtr;
}

uint32_t Chunk::getLod() const
{
	return cbData.bindlessTextureIndex.y;
}

uint32_t Chunk::getTexturePointerFromLod(const uint32_t& lod)
{
	return PtrToModelLOD[lod];
}

Chunk::~Chunk()
{
}
