#pragma once
#include "Graphics.h"
#include "Defines.h"
#include "ConstantBuffers.h"
#include "LightHandler.h"
#include "ResourceManager.h"


class ShadowMaps {
public:
	ShadowMaps();
	~ShadowMaps();
	void init(Graphics* gfx, ResourceManager* rm);
	void update(LightHandler& lightHandler);
	void setAsDepthPass();
	void setAsShadowPass();
	void SetAsDepthStencil(const uint32_t index);
	void clearDeapthStencils();
private:
	void createDepthStencils();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> depthBufferDescriptorHeap;

	TextureViewClass* shadowMapTextureViewClass[MAXNROFLIGHTS];//gfx should delete these
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	D3D12_RESOURCE_STATES state;
	Graphics* gfx;
	ResourceManager* rm;

	uint32_t nrOfLightsTaken = 0;
	uint32_t nrOfActiveShadowMaps = 0;
};