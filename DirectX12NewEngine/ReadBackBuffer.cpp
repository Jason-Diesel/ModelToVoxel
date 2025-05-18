#include "ReadBackBuffer.h"
#include "readImage.h"

ReadBackBuffer::ReadBackBuffer()
{
}

ReadBackBuffer::ReadBackBuffer(
    void* data,
    const uint32_t sizeOfData,
    Graphics* gfx
)
{
    init(data, sizeOfData, gfx);
}

ReadBackBuffer::~ReadBackBuffer()
{
    readbackBuffer->Release();
}

void ReadBackBuffer::init(void* data, const uint32_t sizeOfData, Graphics* gfx)
{
    this->sizeOfData = sizeOfData;
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeOfData);
    CD3DX12_HEAP_PROPERTIES readbackHeapProps(D3D12_HEAP_TYPE_READBACK);

    D3D12_RESOURCE_STATES state = data != nullptr ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    gfx->getDevice()->CreateCommittedResource(
        &readbackHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        state,
        nullptr,
        IID_PPV_ARGS(&readbackBuffer)
    );

    if (data == nullptr)
    {
        return;
    }

    ID3D12Resource* uploadBuffer;
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);

    gfx->getDevice()->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );

    CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferDesc2 = CD3DX12_RESOURCE_DESC::Buffer(sizeOfData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    gfx->getDevice()->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc2,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&gpuBuffer)
    );

    void* mappedData;
    uploadBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, data, sizeOfData);
    uploadBuffer->Unmap(0, nullptr);

    CheckHR(gfx->getCommandAllocator(1)->Reset())
        CheckHR(gfx->getCommandList(1)->Reset(gfx->getCommandAllocator(1), nullptr))

        //FROM STATE COMMON TO COPY DEST OR I GET A WARNING / ERROR (GPU BASED VALIDATION)
        CD3DX12_RESOURCE_BARRIER transitionToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            gpuBuffer,
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_COPY_DEST
        );
    gfx->getCommandList(1)->ResourceBarrier(1, &transitionToCopyDest);

    //COPY DATA FROM CPU TO GPU
    gfx->getCommandList(1)->CopyBufferRegion(readbackBuffer, 0, uploadBuffer, 0, sizeOfData);
    gfx->getCommandList(1)->CopyBufferRegion(gpuBuffer, 0, uploadBuffer, 0, sizeOfData);

    //MAKE GPU BUFFER UAV SO WE CAN SEND IT TO THE GPU
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        gpuBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS // For compute shader use
    );
    gfx->getCommandList(1)->ResourceBarrier(1, &barrier);

    CheckHR(gfx->getCommandList(1)->Close())
        ID3D12CommandList* const commandLists[] = { gfx->getCommandList(1) };
    gfx->getCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);
    CheckHR(gfx->getCommandQueue()->Signal(gfx->getFence(), ++gfx->getFenceValue()))
        CheckHR(gfx->getFence()->SetEventOnCompletion(gfx->getFenceValue(), gfx->getFenceEvent()))
        if (::WaitForSingleObject(gfx->getFenceEvent(), 2000) == WAIT_FAILED)
        {
            breakDebug;
        }

    uploadBuffer->Release();
}

ID3D12Resource* ReadBackBuffer::getResource()
{
    return readbackBuffer;
}

ID3D12Resource* ReadBackBuffer::getUAVResource()
{
    return gpuBuffer;
}

