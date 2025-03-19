#include "Graphics.h"
#include "ConstantBuffers.h"

Graphics::Graphics():
	main_device(nullptr),
	dxgi_factory(nullptr)
{
	
}

Graphics::~Graphics()
{
	commandQueue->Signal(fence.Get(), ++fenceValue);
	fence->SetEventOnCompletion(fenceValue, fenceEvent);
	if (WaitForSingleObject(fenceEvent, 2000) == WAIT_FAILED) {
		breakDebug;
	}
}

void Graphics::init(Window& window)
{
	initInputLayouts();
	setUpDirectX12(window);
	viewPort.Height = (float)window.WindowHeight();
	viewPort.Width = (float)window.WindowWidth();
	viewPort.MaxDepth = 1.0f;

#ifdef _DEBUG
	{
		Microsoft::WRL::ComPtr <ID3D12InfoQueue> infoQueue;
		if (main_device->QueryInterface(IID_PPV_ARGS(&infoQueue)) != S_OK) {
			breakDebug;
		}
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

	}
#endif // DEBUG

	//THIS IS GOOD
	textureHeap.init(MAXNROFMATERIALS, this->main_device.Get());
}

void Graphics::setUpDirectX12(Window& window)
{
	if (main_device) {
		exit(-1);
	}

	UINT dxgi_factory_flags = 0;
#ifdef _DEBUG
	{
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugInterface;
		CheckHR(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)))
		debugInterface->EnableDebugLayer();
		debugInterface->SetEnableGPUBasedValidation(TRUE);
	}
	dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	CheckHR(CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&dxgi_factory)))
	
	//Create deveice
	main_adapter.Attach(determineMainAdapter());
	if (!main_adapter.Get())
	{
		breakDebug;
	}

	D3D_FEATURE_LEVEL maxFeatureLevel{ getMaxFeatureLevel(main_adapter.Get()) };
	if (maxFeatureLevel < minumumFeatureLevel) {
		breakDebug;
	}

	CheckHR(D3D12CreateDevice(main_adapter.Get(), maxFeatureLevel, IID_PPV_ARGS(&main_device)))

#ifdef _DEBUG
	main_device->SetName(L"Main_Device");
#endif
	//

	//rtv descriptor heap
	{
		D3D12_COMMAND_QUEUE_DESC desc;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		CheckHR(main_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)))
	}

	{
		DXGI_SWAP_CHAIN_DESC1 desc;
		desc.Width = window.WindowWidth();
		desc.Height = window.WindowHeight();
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Stereo = false;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = NrOfFrameBuffers;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.Flags = 0;// DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;//is this v-sync off?

		Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
		CheckHR(dxgi_factory->CreateSwapChainForHwnd(
			commandQueue.Get(),
			window.getRenderWindow().getHandle(),
			&desc,
			nullptr, nullptr,
			&tempSwapChain
		))
			CheckHR(tempSwapChain.As(&swapChain))
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = NrOfFrameBuffers
		};
		CheckHR(main_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescriptorHeap)))
	}

	rtvDescriptorSize = main_device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV
	);

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (uint32_t i = 0; i < NrOfFrameBuffers; i++)
		{
			CheckHR(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i])))
			main_device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(rtvDescriptorSize);
		}
	}
	//

	{
		const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT,
			window.WindowWidth(), window.WindowHeight(),
			1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);

		const D3D12_CLEAR_VALUE clearValue = {
			.Format = DXGI_FORMAT_D32_FLOAT,
			.DepthStencil = {1.0f, 0}
		};
		CheckHR(main_device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&depthBuffer)
		))
	}

	{
		const D3D12_DESCRIPTOR_HEAP_DESC desc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = 1
		};
		main_device->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(&depthBufferDescriptorHeap)
		);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		dsvHandle = { depthBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };
		main_device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHandle);
	}

	for (int i = 0; i < MaxNumberOfCommandList; i++)
	{
		CheckHR(main_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i])))
		CheckHR(main_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, getCommandAllocator(), nullptr, IID_PPV_ARGS(&commandList[i])))
		CheckHR(getCommandList(i)->Close())
	}

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CheckHR(main_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)))

	fenceValue = 0;
	CheckHR(main_device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))

		fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (!fenceEvent)
	{
		GetLastError();
		breakDebug;
	}
}

void Graphics::initInputLayouts()
{
	const std::vector<D3D12_INPUT_ELEMENT_DESC> defLayout = {
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"Bitangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
	const std::vector<D3D12_INPUT_ELEMENT_DESC> defSkeletalLayout = {
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"Bitangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

		{"BoneIds", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BoneWeights", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
	const std::vector<D3D12_INPUT_ELEMENT_DESC> voxelLayout = {
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	InputLayouts.push_back(defLayout);
	InputLayouts.push_back(defSkeletalLayout);
	InputLayouts.push_back(voxelLayout);
}

D3D_FEATURE_LEVEL Graphics::getMaxFeatureLevel(IDXGIAdapter4* main_adapter)
{
	constexpr D3D_FEATURE_LEVEL featureLevels[]{
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_12_1,
	};
	D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo{};
	featureLevelInfo.NumFeatureLevels = _countof(featureLevels);
	featureLevelInfo.pFeatureLevelsRequested = featureLevels;

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	CheckHR(D3D12CreateDevice(main_adapter, minumumFeatureLevel, IID_PPV_ARGS(&device)))
	CheckHR(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo)))

	return featureLevelInfo.MaxSupportedFeatureLevel;
}

IDXGIAdapter4* Graphics::determineMainAdapter()
{
	IDXGIAdapter4* adapter = nullptr;

	for (uint32_t i = 0;
		dxgi_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		if (SUCCEEDED(D3D12CreateDevice(adapter, minumumFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
			return adapter;
		}
		if (adapter)
		{
			adapter->Release();
			adapter = nullptr;
		}
	}

	return nullptr;
}

ID3D12DescriptorHeap* Graphics::IMGUIgetDescriptorHeap()
{
	return g_pd3dSrvDescHeap.Get();
}

void Graphics::beginFrame()
{
	currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
	Microsoft::WRL::ComPtr<ID3D12Resource>& backBuffer = backBuffers[currentBackBufferIndex];

	CheckHR(getCommandAllocator()->Reset())
	CheckHR(getCommandList()->Reset(getCommandAllocator(), nullptr))
		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv {
			rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			(int)currentBackBufferIndex, rtvDescriptorSize
		};

	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		getCommandList()->ResourceBarrier(1, &barrier);
		getCommandList()->ClearRenderTargetView(rtv, backGroundColor, 0, nullptr);
		getCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	//SHALL THIS BE BEFORE?
	getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	getCommandList()->RSSetViewports(1, &viewPort);
	getCommandList()->RSSetScissorRects(1, &scissorRect);	
}

void Graphics::endFrame()
{
	fenceMutex.lock();
	Microsoft::WRL::ComPtr<ID3D12Resource>& backBuffer = backBuffers[currentBackBufferIndex];
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
		);
		getCommandList()->ResourceBarrier(1, &barrier);
	}
	{
		CheckHR(getCommandList()->Close())
		ID3D12CommandList* const commandLists[] = { getCommandList() };
		commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	}

	CheckHR(commandQueue->Signal(fence.Get(), ++fenceValue))
	CheckHR(swapChain->Present(0, 0))
	CheckHR(fence->SetEventOnCompletion(fenceValue, fenceEvent))
	if (::WaitForSingleObject(fenceEvent, 2000) == WAIT_FAILED)
	{
		breakDebug;
	}
	fenceMutex.unlock();
}

void Graphics::setNormalRenderTarget()
{
	//WE ARE DOING EXTRA JOBB HERE
	const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{
		rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		(int)currentBackBufferIndex, rtvDescriptorSize
	};
	getCommandList()->OMSetRenderTargets(1, &rtv, FALSE, &dsvHandle);
}

ID3D12GraphicsCommandList* Graphics::getCommandList(int WhatList)
{
	return this->commandList[WhatList].Get();
}

ID3D12CommandAllocator* Graphics::getCommandAllocator(int WhatAllocator)
{
	return this->commandAllocator[WhatAllocator].Get();
}

ID3D12CommandQueue* Graphics::getCommandQueue()
{
	return this->commandQueue.Get();
}

ID3D12Device8* Graphics::getDevice()
{
	return this->main_device.Get();
}

HANDLE& Graphics::getFenceEvent()
{
	return this->fenceEvent;
}

ID3D12Fence* Graphics::getFence()
{
	return this->fence.Get();
}

uint64_t& Graphics::getFenceValue()
{
	return this->fenceValue;
}

const std::vector<D3D12_INPUT_ELEMENT_DESC>& Graphics::getInputLayout(const int InputLayoutIndex)
{
	return InputLayouts[InputLayoutIndex];
}

TextureHeap& Graphics::getTextureHeap()
{
	return this->textureHeap;
}

void Graphics::setCurrentTextureHeap()
{
	ID3D12DescriptorHeap* heaps[] = { textureHeap.getHeap() };
	getCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
	getCommandList()->SetGraphicsRootDescriptorTable(4, textureHeap.getHeap()->GetGPUDescriptorHandleForHeapStart());
}

void Graphics::setCurrentTextureHeap(TextureHeap& texHeap)
{
	ID3D12DescriptorHeap* heaps[] = { texHeap.getHeap() };
	getCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
	getCommandList()->SetGraphicsRootDescriptorTable(4, texHeap.getHeap()->GetGPUDescriptorHandleForHeapStart());
}

void Graphics::setNormalViewPort()
{
	getCommandList()->RSSetViewports(1, &viewPort);
}
