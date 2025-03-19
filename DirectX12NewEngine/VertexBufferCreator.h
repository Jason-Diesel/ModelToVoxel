#pragma once
#include "CommonHeaders.h"

ID3D12Resource* createIndeciesBuffer(
	const std::vector<uint32_t>& indecies,
	ID3D12Device8* device,
	ID3D12GraphicsCommandList* commandList,
	ID3D12CommandAllocator* commandAllocator,
	ID3D12CommandQueue* commandQueue,
	HANDLE& fenceEvent,
	ID3D12Fence* fence,
	uint64_t& fenceValue);

template <typename T>
ID3D12Resource* createVertexBuffer(
	std::vector<T> vertecies,
	ID3D12Device8* device,
	ID3D12GraphicsCommandList* commandList,
	ID3D12CommandAllocator* commandAllocator,
	ID3D12CommandQueue* commandQueue,
	HANDLE& fenceEvent,
	ID3D12Fence* fence,
	uint64_t& fenceValue
)
{
	ID3D12Resource* vertexBuffer = nullptr;
	uint32_t nrOfVertecies = (uint32_t)vertecies.size();
	uint32_t verteciesSize = nrOfVertecies * sizeof(T);

	{
		const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(verteciesSize);
		CheckHR(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&vertexBuffer)
		))
	}
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexUploadBuffer;
	{
		const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(verteciesSize);
		CheckHR(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexUploadBuffer)
		))
	}
	{
		void* mappedVertexData = nullptr;
		CD3DX12_RANGE readRange(0, 0);
		CheckHR(vertexUploadBuffer->Map(0, &readRange, &mappedVertexData));
		memcpy(mappedVertexData, vertecies.data(), verteciesSize);
		vertexUploadBuffer->Unmap(0, nullptr);
	}

	CheckHR(commandAllocator->Reset())
	CheckHR(commandList->Reset(commandAllocator, nullptr))
	commandList->CopyResource(vertexBuffer, vertexUploadBuffer.Get());
	{
		const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			vertexBuffer,
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);
		commandList->ResourceBarrier(1, &barrier);
	}
	CheckHR(commandList->Close())
	ID3D12CommandList* const commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	CheckHR(commandQueue->Signal(fence, ++fenceValue))
	CheckHR(fence->SetEventOnCompletion(fenceValue, fenceEvent))
	if (WaitForSingleObject(fenceEvent, 2000) == WAIT_FAILED)
	{
		breakDebug;
	}

	return vertexBuffer;
}