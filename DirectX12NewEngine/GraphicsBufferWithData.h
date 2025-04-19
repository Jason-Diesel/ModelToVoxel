#pragma once
#include "Graphics.h"

struct GraphicsBufferWithData {
	GraphicsBufferWithData();
	void reset();
	void init(
		void* data, 
		const uint32_t sizeOfData, 
		Graphics* gfx,
		const D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON);
	ID3D12Resource* resource;
	uint32_t posInHeap;
};