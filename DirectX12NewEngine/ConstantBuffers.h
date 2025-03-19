#pragma once
#include "Graphics.h"
#include "Defines.h"

//ConstantBuffers B0 to B3 are occepied
struct ConstantBuffer {
	ID3D12Resource* constantBuffer;
	uint32_t dataSize;
	uint8_t* cbvDataBegin = nullptr;
};

//B0 for camera stuff
struct ViewProjConstantBufferData {
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projectionMatrix;
	DirectX::XMFLOAT4 cameraPosition;
};

//B1 for position of the mesh
struct TransformConstantBuffer {
	DirectX::XMMATRIX transform;
};

//B2 material data
struct MaterialConstantBuffer {
	//EMPTY FOR NOW
	UINT materialIndex;
};

//B3
struct LightConstantBufferData
{
	uint32_t nrOfLights;
	DirectX::XMMATRIX ViewProjectionMatrix[MAXNROFLIGHTS];
	DirectX::XMINT4 ShadowMapInfo[MAXNROFLIGHTS];//X = Shadowmap index in texture array, Y = ShadowSoftness 0 = hard, zw Not defined yet
	DirectX::XMFLOAT4 LightPosoLightType[MAXNROFLIGHTS];//(xyz) = lightPos, w = lightType
	DirectX::XMFLOAT4 LightColor[MAXNROFLIGHTS]; //(xyzw) = RGBA
};

//B4
struct SkeletalAnimationConstantBuffer
{
	DirectX::XMMATRIX SkeletalMatrix[65];
};

template<typename T>
ConstantBuffer CreateConstantBuffer(Graphics* gfx, const T& ConstantBufferData = T())
{
	ConstantBuffer constantBuffer;
	constantBuffer.dataSize = (sizeof(T) + 255) & ~255;
	
	if (constantBuffer.dataSize % 256 != 0)
	{
		std::cout << "Constant buffer size must be a multiple of 256 bytes. is Currently " << sizeof(T) << std::endl;
		breakDebug;
		return ConstantBuffer();
	}

	//ID3D12Resource* constantBuffer;
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBuffer.dataSize);

	CheckHR(gfx->getDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&constantBuffer.constantBuffer)
	))
		

	CD3DX12_RANGE readRange(0, 0); // Do not intend to read from this resource on the CPU
	CheckHR(constantBuffer.constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&constantBuffer.cbvDataBegin)))
	memcpy(constantBuffer.cbvDataBegin, &ConstantBufferData, sizeof(ConstantBufferData));
	constantBuffer.constantBuffer->Unmap(0, nullptr);

	return constantBuffer;
}

//Create a constantbuffer based on size and no data is given
ConstantBuffer CreateConstantBuffer_S(Graphics* gfx, const uint32_t& size);

template<typename T>
void updateConstantBuffer(const T& data, ConstantBuffer& constantBuffer)
{
	memcpy(constantBuffer.cbvDataBegin, &data, constantBuffer.dataSize);
}

void updateConstantBuffer(const void* data, const uint32_t dataSize, ConstantBuffer& constantBuffer);

template<typename T>
void updateConstantBufferFrom(const uint32_t offset, const T& data, ConstantBuffer& constantBuffer)
{
	memcpy(constantBuffer.cbvDataBegin + offset, &data, sizeof(T));
}

void updateConstantBufferFrom(const uint32_t offset, const void* data, const uint32_t dataSize, ConstantBuffer& constantBuffer);