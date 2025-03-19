#include "readImage.h"

static const std::string ExtraNamingTexture = "Texture_Resource_";

void readImage(
	TextureViewClass* theReturn, 
	const std::string& filePath,
	ResourceManager* rm,
	Graphics* gfx,
	D3D12_RESOURCE_FLAGS resourceFlags,
	D3D12_RESOURCE_STATES resourceState
) 
{
	DirectX::ScratchImage image;

	HRESULT hr = S_OK;
	if (filePath.substr(filePath.size() - 3) == "tga")
	{
		hr = DirectX::LoadFromTGAFile(createStringToWString(filePath).c_str(), nullptr, image);
	}
	else {
		hr = DirectX::LoadFromWICFile(createStringToWString(filePath).c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
	}

	if (FAILED(hr)) {
		delete theReturn;
		return;
	}

	DirectX::ScratchImage* mipChain = new DirectX::ScratchImage();
	hr = DirectX::GenerateMipMaps(*image.GetImages(), DirectX::TEX_FILTER_LINEAR, 0, *mipChain);
	if (hr != S_OK)
	{
		delete mipChain;
		mipChain = &image;
	}
	{
		const auto& chainBase = *image.GetImages();
		const D3D12_RESOURCE_DESC texDesc{
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,//Need to make a function where instead of an image file we have raw data
			.Width = (UINT)chainBase.width,
			.Height = (UINT)chainBase.height,
			.DepthOrArraySize = 1,
			.MipLevels = (UINT16)mipChain->GetImageCount(),
			.Format = chainBase.format,
			.SampleDesc = {.Count = 1 },
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
			.Flags = resourceFlags
		};

		const CD3DX12_HEAP_PROPERTIES heapProbs{ D3D12_HEAP_TYPE_DEFAULT };
		CheckHR(gfx->getDevice()->CreateCommittedResource(
			&heapProbs,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&theReturn->srvResource)
		))
	}

	const auto subresourceData = std::ranges::views::iota(0, (int)mipChain->GetImageCount()) |
		std::ranges::views::transform([&](int i) {
		const auto img = mipChain->GetImage(i, 0, 0);
		return D3D12_SUBRESOURCE_DATA{
			.pData = img->pixels,
			.RowPitch = (LONG_PTR)img->rowPitch,
			.SlicePitch = (LONG_PTR)img->slicePitch,
		};
			});
	std::vector<D3D12_SUBRESOURCE_DATA> subresourceVector(subresourceData.begin(), subresourceData.end());

	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
	{
		const CD3DX12_HEAP_PROPERTIES heapProbs{ D3D12_HEAP_TYPE_UPLOAD };
		const auto uploadBufferSize = GetRequiredIntermediateSize(
			theReturn->srvResource.Get(), 0, (UINT)subresourceVector.size()
		);
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		CheckHR(gfx->getDevice()->CreateCommittedResource(
			&heapProbs,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuffer)
		))
	}

	CheckHR(gfx->getCommandAllocator()->Reset())
	CheckHR(gfx->getCommandList()->Reset(gfx->getCommandAllocator(), nullptr))

	UpdateSubresources(
		gfx->getCommandList(),
		theReturn->srvResource.Get(),
		uploadBuffer.Get(),
		0, 0,
		(UINT)subresourceVector.size(),
		subresourceVector.data()
	);

	{
		const CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			theReturn->srvResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, resourceState
		);
		gfx->getCommandList()->ResourceBarrier(1, &Barrier);
	}
	{
		CheckHR(gfx->getCommandList()->Close())
		ID3D12CommandList* const commandLists[] = { gfx->getCommandList() };
		gfx->getCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);
	}
	CheckHR(gfx->getCommandQueue()->Signal(gfx->getFence(), ++gfx->getFenceValue()))
	CheckHR(gfx->getFence()->SetEventOnCompletion(gfx->getFenceValue(), gfx->getFenceEvent()))
	if (::WaitForSingleObject(gfx->getFenceEvent(), 2000) == WAIT_FAILED)
	{
		breakDebug;
	}

	theReturn->textureType = D3D12_SRV_DIMENSION_TEXTURE2D;
	delete mipChain;
	rm->addResource(theReturn, ExtraNamingTexture + filePath);
}

void CreateImage(
	TextureViewClass* theReturn,
	void* data,							//The data
	const DirectX::XMINT3& size,		//Sizes of the image
	const uint32_t dataSize,			//Size of the Type
	ResourceManager* rm,				//RM
	Graphics* gfx,						//GFX
	D3D12_RESOURCE_FLAGS resourceFlags,	//D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS or D3D12_RESOURCE_FLAG_NONE
	D3D12_RESOURCE_STATES resourceState,//D3D12_RESOURCE_STATE_UNORDERED_ACCESS or D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE or D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE
	DXGI_FORMAT format,					//ex : DXGI_FORMAT_R32G32B32A32_FLOAT
	bool threeD							//If it's in 3D or 2D
)
{
	const D3D12_RESOURCE_DESC texDesc = {
		.Dimension = threeD ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = static_cast<UINT>(size.x),
		.Height = static_cast<UINT>(size.y),
		.DepthOrArraySize = static_cast<UINT16>(threeD ? size.z : 1),
		.MipLevels = 1,
		.Format = format,
		.SampleDesc = {1, 0},
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = resourceFlags
	};

	const CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	CheckHR(gfx->getDevice()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		resourceFlags == D3D12_RESOURCE_STATE_UNORDERED_ACCESS ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&theReturn->srvResource)
	))

	// Create the upload heap
	Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;
	UINT64 uploadBufferSize;
	gfx->getDevice()->GetCopyableFootprints(
		&texDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize
	);

	{
		const CD3DX12_HEAP_PROPERTIES heapPropsUpload(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

		CheckHR(gfx->getDevice()->CreateCommittedResource(
			&heapPropsUpload,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap)
		));
	}

	// Map the upload heap and copy the texture data to it
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = data;
	subresourceData.RowPitch = size.x * dataSize;
	subresourceData.SlicePitch = subresourceData.RowPitch * size.y;

	CheckHR(gfx->getCommandAllocator(1)->Reset())
	CheckHR(gfx->getCommandList(1)->Reset(gfx->getCommandAllocator(1), nullptr))
	UpdateSubresources(gfx->getCommandList(1), theReturn->srvResource.Get(), textureUploadHeap.Get(), 0, 0, 1, &subresourceData);

	// Transition the texture to PIXEL_SHADER_RESOURCE state
	const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		theReturn->srvResource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, resourceState
	);
	gfx->getCommandList(1)->ResourceBarrier(1, &barrier);

	// Close and execute the command list
	CheckHR(gfx->getCommandList(1)->Close());
	ID3D12CommandList* const commandLists[] = { gfx->getCommandList(1) };
	gfx->getCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

	// Wait for GPU to finish executing
	CheckHR(gfx->getCommandQueue()->Signal(gfx->getFence(), ++gfx->getFenceValue()));
	CheckHR(gfx->getFence()->SetEventOnCompletion(gfx->getFenceValue(), gfx->getFenceEvent()));
	if (WaitForSingleObject(gfx->getFenceEvent(), 2000) == WAIT_FAILED)
	{
		throw std::runtime_error("WaitForSingleObject failed");
	}

	theReturn->textureType = threeD ? D3D12_SRV_DIMENSION_TEXTURE3D : D3D12_SRV_DIMENSION_TEXTURE2D;
	theReturn->UAVType = threeD ? D3D12_UAV_DIMENSION_TEXTURE3D : D3D12_UAV_DIMENSION_TEXTURE2D;
}

TextureViewClass* createTexture(
	const std::string& filePath, 
	ResourceManager* rm,
	Graphics* gfx
)
{
	struct stat buffer;
	if (!(stat(filePath.c_str(), &buffer) == 0)) {
		std::cout << "Error couldn't find texture" << std::endl;
		return nullptr;
	}

	TextureViewClass* theReturn;

	theReturn = rm->getResource<TextureViewClass>(ExtraNamingTexture + filePath);
	if (theReturn != nullptr)
	{
		return theReturn;
	}

	theReturn = new TextureViewClass();

	//readImage(theReturn, filePath, rm, gfx, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	readImage(theReturn, filePath, rm, gfx, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

	return theReturn;
}

TextureViewClass* createTextureWithWriteAccess(const std::string& filePath, ResourceManager* rm, Graphics* gfx)
{
	struct stat buffer;
	if (!(stat(filePath.c_str(), &buffer) == 0)) {
		std::cout << "Error couldn't find texture" << std::endl;
		return nullptr;
	}

	TextureViewClass* theReturn;

	theReturn = rm->getResource<TextureViewClass>(ExtraNamingTexture + filePath);
	if (theReturn != nullptr)
	{
		std::cout << "texture : " << filePath << " already exist, if you wanna change it, delete it from rm, and reread it" << std::endl;
		return theReturn;
	}

	theReturn = new TextureViewClass();
	readImage(theReturn, filePath, rm, gfx, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return theReturn;
}


TextureViewClass* createUAV(void* data, const uint32_t sizeofType, const DirectX::XMINT2& size, ResourceManager* rm, Graphics* gfx, const DXGI_FORMAT format)
{
	TextureViewClass* returnData = new TextureViewClass();
	CreateImage(
		returnData,
		data,
		DirectX::XMINT3(size.x, size.y, 1),
		sizeofType,
		rm,
		gfx,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		format,
		false
	);
	return returnData;
}

TextureViewClass* createTexture(void* data, const uint32_t sizeofType, const DirectX::XMINT2& size, ResourceManager* rm, Graphics* gfx, const DXGI_FORMAT format)
{
	TextureViewClass* returnData = new TextureViewClass();
	CreateImage(
		returnData,
		data,
		DirectX::XMINT3(size.x, size.y, 1),
		sizeofType,
		rm,
		gfx,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		format,
		false
	);
	return returnData;
}

TextureViewClass* createTexture(void* data, const uint32_t sizeofType, const DirectX::XMINT3& size, ResourceManager* rm, Graphics* gfx, const DXGI_FORMAT format)
{
	TextureViewClass* returnData = new TextureViewClass();
	CreateImage(
		returnData,
		data,
		DirectX::XMINT3(size.x, size.y, size.z),
		sizeofType,
		rm,
		gfx,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		format,
		true
	);
	return returnData;
}