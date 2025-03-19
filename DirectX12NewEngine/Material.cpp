#include "Material.h"
#include "HelperFuncitons.h"

Material::Material()
{
}

Material::Material(Graphics* gfx)
{
	constantBuffer = CreateConstantBuffer<int32_t>(gfx);//TODO : This needs to be redone?
}

Material::Material(
	Graphics* gfx, 
	const std::vector<uint32_t>& textures, 
	MaterialType materialTypeFlag, 
	uint32_t sizeOfConstantBufferData, 
	void* data
)
{
	textureTypeAndIndex.reserve(textures.size());
	offset = sizeOfConstantBufferData;
	materialDataSize = sizeOfConstantBufferData;
	constantBuffer = CreateConstantBuffer_S(gfx, (textures.size() * sizeof(uint32_t)) + sizeOfConstantBufferData);
	for (int i = 0; i < textures.size(); i++)
	{
		textureTypeAndIndex.push_back(std::pair<uint32_t, uint32_t>(i, textures[i]));
	}

	updateConstantBuffer(data, sizeOfConstantBufferData, constantBuffer);
	this->finalize((int)materialTypeFlag);
}

Material::~Material()
{
	constantBuffer.constantBuffer->Release();
}

void Material::setMaterial(Graphics* gfx)
{
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(2, constantBuffer.constantBuffer->GetGPUVirtualAddress());
}

void Material::init(const uint32_t& nrOfTextures)
{
	textureTypeAndIndex.reserve(nrOfTextures);
}

void Material::init(Graphics* gfx, const uint32_t& nrOfTextures, const uint32_t& sizeOfData)
{
	textureTypeAndIndex.reserve(nrOfTextures);
	offset = sizeOfData;
	materialDataSize = sizeOfData;
	constantBuffer = CreateConstantBuffer_S(gfx, (nrOfTextures * sizeof(uint32_t)) + sizeOfData);
}

void Material::addTexture(const uint32_t& pos, TextureViewClass* texture, Graphics* gfx, int numberOfMips)
{
	textureTypeAndIndex.push_back(std::pair<uint32_t, uint32_t>(pos, gfx->getTextureHeap().createSRV(texture, gfx, numberOfMips)));
}

void Material::setTexture(const uint32_t& pos, const uint32_t& texturePtr)
{
	textureTypeAndIndex.push_back(std::pair<uint32_t, uint32_t>(pos, texturePtr));
}

void Material::finalize(const int32_t materialFlag)
{
	this->materialFlag = materialFlag;

	std::vector<uint32_t> textures;
	textures.reserve(textureTypeAndIndex.size());
	for (int i = 0; i < textureTypeAndIndex.size(); i++)
	{
		textures.push_back(textureTypeAndIndex[i].second);
	}
	//sizeof(int32_t) * 4 is from material flag beacuse shaders need every value to be a multiplication of 16 for some reason
	//The rest is the texture index
	uint32_t dataSize = sizeof(int32_t) * 4 + sizeof(uint32_t) * textureTypeAndIndex.size();//Where did sizeof(int32_t) * 4 come from?

	if (dataSize > 48)
	{
		uint8_t* data = (uint8_t*)malloc(dataSize);
		memcpy(data, &materialFlag, sizeof(int32_t));
		memcpy(&data[16], textures.data(), sizeof(uint32_t) * textureTypeAndIndex.size());

		updateConstantBufferFrom(offset, data, dataSize, constantBuffer);

		free(data);
	}
	else 
	{
		static uint8_t data[48] = {};
		memcpy(data, &materialFlag, sizeof(int32_t));
		memcpy(data + 16, textures.data(), sizeof(uint32_t) * textureTypeAndIndex.size());
		updateConstantBufferFrom(offset, &data, dataSize, constantBuffer);
	}
}

const uint32_t Material::getNrOfTextures()
{
	return this->textureTypeAndIndex.size();
}
