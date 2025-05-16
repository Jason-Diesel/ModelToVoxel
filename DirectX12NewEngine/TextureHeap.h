#pragma once
#include "CommonHeaders.h"
#include "Texture.h"
#include <unordered_map>

class Graphics;
class ReadBackBuffer;

class TextureHeap
{
public:
	TextureHeap();
	~TextureHeap();
	void init(const uint32_t& nrOfDescriptors, ID3D12Device8* device);
	void reset();

	uint32_t MAXNROFSRV;
	uint32_t createSRV(const uint32_t pos, TextureViewClass* texture, ID3D12Device8* device, int numberOfMips = 1);
	uint32_t createSRV(TextureViewClass* texture, Graphics* gfx, int numberOfMips = 1);
	uint32_t createSRVShadow(TextureViewClass* texture, Graphics* gfx, int numberOfMips = 1);

	uint32_t createUAV(TextureViewClass* texture, Graphics* gfx);
	uint32_t createUAV(const uint32_t pos, TextureViewClass* texture, ID3D12Device8* device);
	uint32_t createUAV(
		ID3D12Resource* resource, 
		ID3D12Device8* device,
		const uint32_t nrOfElements,
		const uint32_t sizeOfElement
	);

	uint32_t createNormalResource(
		ID3D12Resource* resource, 
		ID3D12Device8* device,
		const uint32_t sizeofType = sizeof(uint32_t),
		const uint32_t nummerOfelements = 1
	);

	//Doesn't delete the resource
	bool removeFromHeap(const uint32_t pos);
	//Deletes the resource
	bool deleteFromHeap(const uint32_t pos);

	uint32_t addReadBackBuffer(ReadBackBuffer* rbBuffer, Graphics* gfx);

	ID3D12DescriptorHeap* getHeap();
	TextureViewClass** TextureViewptrs;//An array of textureViewClassPointers
private:
	UINT DescriptorSize;
	uint32_t nrOfCurrentTextures;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
	std::unordered_map<TextureViewClass*, uint32_t> texturePointer;
	TextureViewClass* UAVThatDoesntExist;
};