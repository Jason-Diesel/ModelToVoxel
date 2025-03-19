#include "ConstantBuffers.h"

ConstantBuffer CreateConstantBuffer_S(Graphics* gfx, const uint32_t& size)
{
	ConstantBuffer constantBuffer;
	constantBuffer.dataSize = (size + 255) & ~255;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBuffer.dataSize);

	CheckHR(gfx->getDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&constantBuffer.constantBuffer)
	))

		CD3DX12_RANGE readRange(0, 0); // Do not intend to read from this resource on the CPU
	CheckHR(constantBuffer.constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&constantBuffer.cbvDataBegin)))

		return constantBuffer;
}
void updateConstantBuffer(const void* data, const uint32_t dataSize, ConstantBuffer& constantBuffer)
{
	memcpy(constantBuffer.cbvDataBegin, data, dataSize);
}

void updateConstantBufferFrom(const uint32_t offset, const void* data, const uint32_t dataSize, ConstantBuffer& constantBuffer)
{
	if (offset + dataSize > constantBuffer.dataSize) {
		std::cerr << "Error: Attempt to write beyond the bounds of the constant buffer!" << std::endl;
		return;
	}
	uint8_t* p = constantBuffer.cbvDataBegin;
	memcpy(p + offset, data, dataSize);
	//memcpy(constantBuffer.cbvDataBegin + offset, data, dataSize);
}
