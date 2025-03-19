#pragma once
#include "CommonHeaders.h"
#include "WindowClass.h"
#include <mutex>
#include "TextureHeap.h"

struct ConstantBuffer;
struct MaterialConstantBuffer;

constexpr D3D_FEATURE_LEVEL minumumFeatureLevel{ D3D_FEATURE_LEVEL_11_0 };

class Graphics
{
public:
	Graphics();
	~Graphics();

	void init(Window& window);
	
	void update(float dt);
	void updateWindow();

	void beginFrame();
	void endFrame();
	void setNormalRenderTarget();
public:
	ID3D12GraphicsCommandList* getCommandList(int WhatList = 0);
	ID3D12CommandAllocator* getCommandAllocator(int WhatAllocator = 0);
	ID3D12CommandQueue* getCommandQueue();
	ID3D12Device8* getDevice();
	HANDLE& getFenceEvent();
	ID3D12Fence* getFence();
	uint64_t& getFenceValue();
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& getInputLayout(const int InputLayoutIndex);
	TextureHeap& getTextureHeap();
	void setCurrentTextureHeap();
	void setCurrentTextureHeap(TextureHeap& texHeap);
	void setNormalViewPort();
private:
	Microsoft::WRL::ComPtr<ID3D12Device8> main_device;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgi_factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> main_adapter;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers[NrOfFrameBuffers];

	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> depthBufferDescriptorHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	//
	static const int MaxNumberOfCommandList = 2;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator[MaxNumberOfCommandList];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList[MaxNumberOfCommandList];

	std::mutex fenceMutex;
	//
	uint32_t currentBackBufferIndex;
	uint32_t rtvDescriptorSize;

	HANDLE fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	uint64_t fenceValue;


	TextureHeap textureHeap;
private:
	float fov = 60;
	//Window window;
	const CD3DX12_RECT scissorRect{ 0,0, LONG_MAX, LONG_MAX };
	CD3DX12_VIEWPORT viewPort;
	float backGroundColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

	//shaders
	std::vector<std::vector<D3D12_INPUT_ELEMENT_DESC>> InputLayouts;//make a map maybe?
private:
	void setUpDirectX12(Window& window);
	void initInputLayouts();
	//helperfunctions
	D3D_FEATURE_LEVEL getMaxFeatureLevel(IDXGIAdapter4* main_adapter);
	IDXGIAdapter4* determineMainAdapter();
private:
	//IMGUI
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> g_pd3dSrvDescHeap;
public:
	ID3D12DescriptorHeap* IMGUIgetDescriptorHeap();
};

