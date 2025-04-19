#pragma once
#include "ResourceManager.h"
#include "Graphics.h"
#include "CommonHeaders.h"
#include <DirectXTex.h>
#include "HelperFuncitons.h"
#include <ranges>

TextureViewClass* createTexture(
	const std::string& filePath, 
	ResourceManager* rm,
	Graphics* gfx,
	uint32_t commandThread = 0
);

TextureViewClass* createTextureWithWriteAccess(
	const std::string& filePath,
	ResourceManager* rm,
	Graphics* gfx
);

TextureViewClass* createUAV(
	void* data,
	const uint32_t sizeofType,
	const DirectX::XMINT2& size,
	ResourceManager* rm,
	Graphics* gfx,
	const DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT
);

TextureViewClass* createUAV(
	void* data,
	const uint32_t sizeofType,
	const DirectX::XMINT3& size,
	ResourceManager* rm,
	Graphics* gfx,
	const DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT,
	const uint32_t& nrOfMips = 1//Need to use a computeShader to make the things happen
);
TextureViewClass* createEmptyUAV(
	const uint32_t sizeofType,
	const DirectX::XMINT3& size,
	Graphics* gfx,
	const DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT
	);

TextureViewClass* createTexture(
	void* data,
	const uint32_t sizeofType,
	const DirectX::XMINT2& size,
	ResourceManager* rm,
	Graphics* gfx,
	const DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT
);

TextureViewClass* createTexture(
	void* data,
	const uint32_t sizeofType,
	const DirectX::XMINT3& size,
	ResourceManager* rm,
	Graphics* gfx,
	const DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT,
	const uint32_t& nrOfMips = 1
);
