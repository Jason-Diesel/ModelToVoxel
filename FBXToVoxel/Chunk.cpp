#include "Chunk.h"

Chunk::Chunk(Graphics* gfx)
{
	materialIndexBuffer = CreateConstantBuffer<ChunkConstantBuffer>(gfx, ChunkConstantBuffer({0}));
	transformBuffer = CreateConstantBuffer<DirectX::XMMATRIX>(gfx);
}

void Chunk::setConstantBuffers(Graphics* gfx)
{
	//this->setConstantBuffer(gfx);
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(2, materialIndexBuffer.constantBuffer->GetGPUVirtualAddress());
}

void Chunk::updateConstantBuffers()
{
	updateConstantBuffer(cbData, materialIndexBuffer);
}

Chunk::~Chunk()
{
}
