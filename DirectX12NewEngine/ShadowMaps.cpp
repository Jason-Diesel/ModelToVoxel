#include "ShadowMaps.h"

ShadowMaps::ShadowMaps()
{
	srand(time(0));
}

ShadowMaps::~ShadowMaps()
{
	
}

void ShadowMaps::init(Graphics* gfx, ResourceManager* rm)
{
	this->gfx = gfx;
	this->rm = rm;
	createDepthStencils();
}

void ShadowMaps::update(LightHandler& lightHandler)
{
	while (lightHandler.getNrOfActiveLights() > nrOfLightsTaken) {
		nrOfLightsTaken++;

		//We do not wanna create a shadowmap for point light
		if (lightHandler.getLight(nrOfLightsTaken - 1)->getLightType() == LightType::PointLight_E)
		{
			continue;
		}

		const DirectX::XMUINT2 smSize = lightHandler.getLight(nrOfLightsTaken - 1)->getShadowMapSize();

		const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R32_TYPELESS,
			smSize.x, smSize.y, 1,//THIS CAN BE 1 and we can have a size
			0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);

		const D3D12_CLEAR_VALUE clearValue = {
				.Format = DXGI_FORMAT_D32_FLOAT,
				.DepthStencil = {1.0f, 0}
		};

		shadowMapTextureViewClass[nrOfActiveShadowMaps] = new TextureViewClass();
		CheckHR(gfx->getDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(shadowMapTextureViewClass[nrOfActiveShadowMaps]->srvResource.ReleaseAndGetAddressOf())
		))
			state = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2DArray.ArraySize = 1;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		UINT handleIncrementSize = gfx->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		dsvHandle = { depthBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };


		dsvHandle.Offset(nrOfActiveShadowMaps, handleIncrementSize);

		dsvDesc.Texture2DArray.FirstArraySlice = nrOfActiveShadowMaps;

		gfx->getDevice()->CreateDepthStencilView(
			shadowMapTextureViewClass[nrOfActiveShadowMaps]->srvResource.Get(), &dsvDesc, dsvHandle
		);
		shadowMapTextureViewClass[nrOfActiveShadowMaps]->textureType = D3D12_SRV_DIMENSION_TEXTURE2D;
		shadowMapTextureViewClass[nrOfActiveShadowMaps]->srvHandle = dsvHandle;

		lightHandler.getLight(nrOfLightsTaken - 1)->getShadowMapPos() = gfx->getTextureHeap().createSRVShadow(shadowMapTextureViewClass[nrOfActiveShadowMaps], gfx);
		rm->addResource(shadowMapTextureViewClass[nrOfActiveShadowMaps]);

		nrOfActiveShadowMaps++;
	}
}


void ShadowMaps::setAsDepthPass()
{
	if (state != D3D12_RESOURCE_STATE_DEPTH_WRITE && nrOfActiveShadowMaps > 0)
	{
		CD3DX12_RESOURCE_BARRIER toDepthWriteBarriers[MAXNROFLIGHTS];
		for (int i = 0; i < nrOfActiveShadowMaps; i++)
		{
			toDepthWriteBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				shadowMapTextureViewClass[i]->srvResource.Get(),
				state, // Previous state
				D3D12_RESOURCE_STATE_DEPTH_WRITE // Target state
			);
		}
		state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		gfx->getCommandList()->ResourceBarrier(nrOfActiveShadowMaps, toDepthWriteBarriers);
	}
}

void ShadowMaps::setAsShadowPass()
{
	if (state != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE && nrOfActiveShadowMaps > 0)
	{
		CD3DX12_RESOURCE_BARRIER toShaderResourceBarriers[MAXNROFLIGHTS];
		for (int i = 0; i < nrOfActiveShadowMaps; i++)
		{
			toShaderResourceBarriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
				shadowMapTextureViewClass[i]->srvResource.Get(),
				state, // Previous state
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE // Target state
			);
		}
		state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		gfx->getCommandList()->ResourceBarrier(nrOfActiveShadowMaps, toShaderResourceBarriers);
	}
}

void ShadowMaps::SetAsDepthStencil(const uint32_t index)
{
	//CLEAR THE RENDER TARGET LATER
	gfx->getCommandList()->OMSetRenderTargets(0, nullptr, FALSE, &shadowMapTextureViewClass[index]->srvHandle);
}

void ShadowMaps::clearDeapthStencils()
{
	for (int i = 0; i < nrOfActiveShadowMaps; i++)
	{
		gfx->getCommandList()->ClearDepthStencilView(shadowMapTextureViewClass[i]->srvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}

void ShadowMaps::createDepthStencils()
{
	const D3D12_DESCRIPTOR_HEAP_DESC desc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		.NumDescriptors = MAXNROFLIGHTS
	};
	gfx->getDevice()->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(&depthBufferDescriptorHeap)
	);
}