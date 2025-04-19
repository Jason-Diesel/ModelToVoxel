#pragma once
#include "Graphics.h"
#include "ConstantBuffers.h"
#include "MaterialTypes.h"

enum MaterialType{
	DIFFUSE_TEXTURE = 1 << 0,
	NORMAL_TEXTURE = 1 << 1,
	HEIGHT_TEXTURE = 1 << 2,
};

struct MaterialDescription
{
	MaterialDescription(const int nrOfTextures = 1, const D3D12_DESCRIPTOR_RANGE_TYPE rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
		this->NrOfTextures = nrOfTextures;
		this->RangeType = rangeType;
	}
	int NrOfTextures;
	D3D12_DESCRIPTOR_RANGE_TYPE RangeType;
};

#define DS(x) &x, sizeof(x)

class Material
{
public:
	Material();
	Material(Graphics* gfx);
	Material(
		Graphics* gfx,
		const std::vector<uint32_t>& textures,
		MaterialType materialTypeFlag,
		uint32_t sizeOfConstantBufferData,
		void* data
		);
	~Material();
	void init(const uint32_t& nrOfTextures);
	void init(Graphics* gfx, const uint32_t& nrOfTextures, const uint32_t& sizeOfData);
	
	void setMaterial(Graphics* gfx);
	void addTexture(const uint32_t& pos, TextureViewClass* texture, Graphics* gfx, int numberOfMips = 1);
	void setTexture(const uint32_t& pos, const uint32_t& texturePtr);
	void finalize(const int32_t materialFlag);
	const uint32_t getNrOfTextures();

	template <typename T>
	void updateMaterialData(const T &data)
	{
#ifdef _DEBUG
		if (sizeof(T) != materialDataSize)
		{
			std::cout << "error" << std::endl;
		}
#endif
		updateConstantBuffer(&data, sizeof(T), constantBuffer);
	}

private:
	std::vector<std::pair<uint32_t, uint32_t>>	textureTypeAndIndex;
	D3D12_SHADER_VISIBILITY						visibility = D3D12_SHADER_VISIBILITY_PIXEL;

	uint32_t materialDataSize;
	uint32_t offset = 0;
	int32_t materialFlag;

	ConstantBuffer constantBuffer;
};

