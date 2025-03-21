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
	cbData.bindlessTextureIndex.y = lod;
}

uint32_t Chunk::getLod() const
{
	return cbData.bindlessTextureIndex.y;
}

Chunk::~Chunk()
{
}
