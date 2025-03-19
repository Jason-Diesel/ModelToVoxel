#pragma once
#include "Graphics.h"
#include "ResourceManager.h"

class ReadBackBuffer{
public:
	ReadBackBuffer(
		void* data,
		const uint32_t sizeOfData,
		Graphics* gfx
	);
	~ReadBackBuffer();
	template<typename T>
	T* getData(Graphics* gfx) {
		CheckHR(gfx->getCommandAllocator(1)->Reset())
		CheckHR(gfx->getCommandList(1)->Reset(gfx->getCommandAllocator(1), nullptr))

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			gpuBuffer,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_SOURCE
		);
		gfx->getCommandList(1)->ResourceBarrier(1, &barrier);
		gfx->getCommandList(1)->CopyBufferRegion(readbackBuffer, 0, gpuBuffer, 0, sizeOfData);

		CheckHR(gfx->getCommandList(1)->Close())
		ID3D12CommandList* const commandLists[] = { gfx->getCommandList(1) };
		gfx->getCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);
		CheckHR(gfx->getCommandQueue()->Signal(gfx->getFence(), ++gfx->getFenceValue()))
		CheckHR(gfx->getFence()->SetEventOnCompletion(gfx->getFenceValue(), gfx->getFenceEvent()))
		if (::WaitForSingleObject(gfx->getFenceEvent(), 2000) == WAIT_FAILED)
		{
			breakDebug;
		}

		void* processedData;
		readbackBuffer->Map(0, nullptr, &processedData);

		uint32_t arraySize = sizeOfData / sizeof(T);
		T* theReturn = new T[arraySize];
		memcpy(theReturn, processedData, sizeOfData);
		return theReturn;
	}
	ID3D12Resource* getResource();
	ID3D12Resource* getUAVResource();
private:
	ID3D12Resource* readbackBuffer;
	ID3D12Resource* gpuBuffer;
	const uint32_t sizeOfData;
};