#include "TextureChanges.h"

void UAVtoSRV(TextureViewClass* tvc, Graphics* gfx)
{
	CD3DX12_RESOURCE_BARRIER toSRVBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		tvc->srvResource.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, // Previous state
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE // Target state
	);
	gfx->getCommandList()->ResourceBarrier(1, &toSRVBarrier);
}

void SRVtoUAV(TextureViewClass* tvc, Graphics* gfx)
{
	CD3DX12_RESOURCE_BARRIER toSRVBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		tvc->srvResource.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS 
	);
	gfx->getCommandList()->ResourceBarrier(1, &toSRVBarrier);
}
