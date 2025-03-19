#include "VertexBufferCreator.h"

ID3D12Resource* createIndeciesBuffer(
	const std::vector<uint32_t>& indecies, 
	ID3D12Device8* device, 
	ID3D12GraphicsCommandList* commandList, 
	ID3D12CommandAllocator* commandAllocator, 
	ID3D12CommandQueue* commandQueue, 
	HANDLE& fenceEvent, 
	ID3D12Fence* fence, 
	uint64_t& fenceValue
)
{
	ID3D12Resource* indeciesBuffer = nullptr;
	uint32_t nrOfindecies = (uint32_t)indecies.size();
	{
		const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(nrOfindecies * sizeof(uint32_t));
		CheckHR(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&indeciesBuffer)
		))
	}
	Microsoft::WRL::ComPtr<ID3D12Resource> indeciesUploadBuffer;
	{
		const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(nrOfindecies * sizeof(uint32_t));
		CheckHR(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indeciesUploadBuffer)
		))
	}
	{
		uint32_t* mappedIndeciesData = nullptr;
		CheckHR(indeciesUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndeciesData)))
			std::ranges::copy(indecies, mappedIndeciesData);
		indeciesUploadBuffer->Unmap(0, nullptr);
	}

	CheckHR(commandAllocator->Reset())
	CheckHR(commandList->Reset(commandAllocator, nullptr))
	commandList->CopyResource(indeciesBuffer, indeciesUploadBuffer.Get());
	{
		const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			indeciesBuffer,
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER
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

	return indeciesBuffer;
}
