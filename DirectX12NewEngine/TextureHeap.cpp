#include "TextureHeap.h"
#include "Graphics.h"
#include "ReadBackBuffer.h"

TextureHeap::TextureHeap():
	nrOfCurrentTextures(0)
{	
	UAVThatDoesntExist = new TextureViewClass();
}

TextureHeap::~TextureHeap()
{
	//TODO : only delete shadows?(or delete shadows in rm)
	//for (int i = 0; i < MAXNROFSRV; i++)
	//{
	//	if (TextureViewptrs[i] != nullptr)
	//	{
	//		delete TextureViewptrs[i];
	//	}
	//}
	delete[] TextureViewptrs;
	//srvHeap->Release();//Shall I not release this?
}

void TextureHeap::init(const uint32_t& nrOfDescriptors, ID3D12Device8* device)
{
	MAXNROFSRV = nrOfDescriptors;

	TextureViewptrs = new TextureViewClass * [nrOfDescriptors] {nullptr};
	DescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = nrOfDescriptors; // Number of descriptors you need
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
}

uint32_t TextureHeap::createSRV(const uint32_t pos, TextureViewClass* texture, ID3D12Device8* device, int numberOfMips)
{
	if (texturePointer.find(texture) != texturePointer.end())
	{
		return texturePointer[texture];
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = texture->srvResource->GetDesc().Format;
	srvDesc.ViewDimension = texture->textureType;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = numberOfMips;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	//remove(pos);//redo this one?
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), pos, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	device->CreateShaderResourceView(texture->srvResource.Get(), &srvDesc, srvHandle);

	nrOfCurrentTextures++;
	TextureViewptrs[pos] = texture;
	texturePointer.insert(std::pair<TextureViewClass*, uint32_t>(texture, pos));
	return pos;
}

uint32_t TextureHeap::createSRV(TextureViewClass* texture, Graphics* gfx, int numberOfMips)
{
	if (texturePointer.find(texture) != texturePointer.end())
	{
		return texturePointer[texture];
	}
	if (this->nrOfCurrentTextures < MAXNROFSRV)
	{
		return this->createSRV(nrOfCurrentTextures, texture, gfx->getDevice(), numberOfMips);
	}

	for (int i = 0; i < MAXNROFSRV; i++)
	{
		if (TextureViewptrs[i] == nullptr)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = texture->srvResource->GetDesc().Format;
			srvDesc.ViewDimension = texture->textureType;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = numberOfMips;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), i, gfx->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			gfx->getDevice()->CreateShaderResourceView(texture->srvResource.Get(), &srvDesc, srvHandle);

			TextureViewptrs[i] = texture;
			nrOfCurrentTextures++;
			texturePointer.insert(std::pair<TextureViewClass*, uint32_t>(texture, i));
			return i;
		}
	}
	return -1;
}

uint32_t TextureHeap::createSRVShadow(TextureViewClass* texture, Graphics* gfx, int numberOfMips)
{
	for (int i = 0; i < MAXNROFSRV; i++)
	{
		if (TextureViewptrs[i] == nullptr)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = texture->textureType;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = numberOfMips;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), i, gfx->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			gfx->getDevice()->CreateShaderResourceView(texture->srvResource.Get(), &srvDesc, srvHandle);

			TextureViewptrs[i] = texture;
			nrOfCurrentTextures++;
			return i;
		}
	}
	return -1;
}

uint32_t TextureHeap::createUAV(TextureViewClass* texture, Graphics* gfx)
{
	if (texturePointer.find(texture) != texturePointer.end())
	{
		return texturePointer[texture];
	}

	for (int i = 0; i < MAXNROFSRV; i++)
	{
		if (TextureViewptrs[i] == nullptr)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = texture->srvResource->GetDesc().Format;
			uavDesc.ViewDimension = texture->UAVType;
			uavDesc.Texture3D.WSize = -1;

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), i, gfx->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			gfx->getDevice()->CreateUnorderedAccessView(texture->srvResource.Get(), nullptr, &uavDesc, srvHandle);

			TextureViewptrs[i] = texture;
			nrOfCurrentTextures++;
			texturePointer.insert(std::pair<TextureViewClass*, uint32_t>(texture, i));
			return i;
		}
	}
	return -1;
}

uint32_t TextureHeap::createUAV(const uint32_t pos, TextureViewClass* texture, ID3D12Device8* device)
{
	if (texturePointer.find(texture) != texturePointer.end())
	{
		return texturePointer[texture];
	}
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = texture->srvResource->GetDesc().Format;
	uavDesc.ViewDimension = texture->UAVType;
	uavDesc.Texture3D.WSize = -1;
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), pos, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	device->CreateUnorderedAccessView(texture->srvResource.Get(), nullptr, &uavDesc, srvHandle);
	
	TextureViewptrs[pos] = texture;
	nrOfCurrentTextures++;
	texturePointer.insert(std::pair<TextureViewClass*, uint32_t>(texture, pos));
	return pos;
}

uint32_t TextureHeap::createUAV(
	ID3D12Resource* resource, 
	ID3D12Device8* device, 
	const uint32_t nrOfElements,
	const uint32_t sizeOfElement
)
{
	for (int i = 0; i < MAXNROFSRV; i++)
	{
		if (TextureViewptrs[i] == nullptr)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = nrOfElements;
			uavDesc.Buffer.StructureByteStride = sizeOfElement;
			uavDesc.Buffer.CounterOffsetInBytes = 0;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), i, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			device->CreateUnorderedAccessView(resource, nullptr, &uavDesc, srvHandle);

			TextureViewptrs[i] = UAVThatDoesntExist;//how should I do here?
			nrOfCurrentTextures++;
			return i;
		}
	}
	return -1;
}

uint32_t TextureHeap::createNormalResource(ID3D12Resource* resource, ID3D12Device8* device, const uint32_t sizeofType, const uint32_t nummerOfelements)
{
	for (int i = 0; i < MAXNROFSRV; i++)
	{
		if (TextureViewptrs[i] == nullptr)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = nummerOfelements;
			uavDesc.Buffer.StructureByteStride = sizeofType;
			uavDesc.Buffer.CounterOffsetInBytes = 0;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), i, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			device->CreateUnorderedAccessView(resource, nullptr, &uavDesc, srvHandle);

			TextureViewptrs[i] = UAVThatDoesntExist;//how should I do here?
			nrOfCurrentTextures++;
			return i;
		}
	}
	return -1;
}

uint32_t TextureHeap::addReadBackBuffer(ReadBackBuffer* rbBuffer, Graphics* gfx)
{
	for (int i = 0; i < MAXNROFSRV; i++)
	{
		if (TextureViewptrs[i] == nullptr)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = 1;
			uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
			uavDesc.Buffer.CounterOffsetInBytes = 0;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(srvHeap->GetCPUDescriptorHandleForHeapStart(), 0, gfx->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			gfx->getDevice()->CreateUnorderedAccessView(rbBuffer->getResource(), nullptr, &uavDesc, srvHandle);

			TextureViewptrs[i] = UAVThatDoesntExist;//how should I do here?
			nrOfCurrentTextures++;
			return i;
		}
	}
	return -1;
}

bool TextureHeap::removeFromHeap(const uint32_t pos)
{
	if (TextureViewptrs[pos] == UAVThatDoesntExist)
	{
		TextureViewptrs[pos] = nullptr;
		return true;
	}
	if (TextureViewptrs[pos] != nullptr)
	{
		texturePointer.erase(TextureViewptrs[pos]);
		TextureViewptrs[pos] = nullptr;
		return true;
	}
	return false;
}

bool TextureHeap::deleteFromHeap(const uint32_t pos)
{
	if (TextureViewptrs[pos] == UAVThatDoesntExist)
	{
		TextureViewptrs[pos] = nullptr;
		return true;
	}
	if (TextureViewptrs[pos] != nullptr)
	{
		texturePointer.erase(TextureViewptrs[pos]);
		TextureViewptrs[pos]->srvResource.Reset();
		delete TextureViewptrs[pos];
		TextureViewptrs[pos] = nullptr;
		return true;
	}
	return false;
}

ID3D12DescriptorHeap* TextureHeap::getHeap()
{
	return srvHeap.Get();
}
