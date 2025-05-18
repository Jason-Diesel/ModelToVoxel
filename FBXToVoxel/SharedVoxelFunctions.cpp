#include "SharedVoxelFunctions.h"

Model* GetVoxelModel(const int size, const int NrOfBlocks, Graphics* gfx)
{
    {
        Model* theReturnModel = new Model();

        theReturnModel->nrOfSubMeshes = 1;
        theReturnModel->subMeshes = new Mesh[1];

        std::vector<VoxelVertecies> vertecies;
        std::vector<uint32_t> indecies;

        //For X
        for (int i = 0; i < NrOfBlocks + 1; i++)
        {
            indecies.push_back((uint32_t)vertecies.size() + 0);
            indecies.push_back((uint32_t)vertecies.size() + 1);
            indecies.push_back((uint32_t)vertecies.size() + 2);
            indecies.push_back((uint32_t)vertecies.size() + 1);
            indecies.push_back((uint32_t)vertecies.size() + 3);
            indecies.push_back((uint32_t)vertecies.size() + 2);

            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3((float)i * size, 0, 0), DirectX::XMFLOAT3(1, 0, 0) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3((float)i * size, 0, NrOfBlocks * size), DirectX::XMFLOAT3(1, 0, 0) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3((float)i * size, NrOfBlocks * size, 0), DirectX::XMFLOAT3(1, 0, 0) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3((float)i * size, NrOfBlocks * size, NrOfBlocks * size), DirectX::XMFLOAT3(1, 0, 0) }));
        }

        //For y
        for (int i = 0; i < NrOfBlocks + 1; i++)
        {
            indecies.push_back((uint32_t)vertecies.size() + 0);
            indecies.push_back((uint32_t)vertecies.size() + 1);
            indecies.push_back((uint32_t)vertecies.size() + 2);
            indecies.push_back((uint32_t)vertecies.size() + 1);
            indecies.push_back((uint32_t)vertecies.size() + 3);
            indecies.push_back((uint32_t)vertecies.size() + 2);

            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3(0, (float)i * size, 0), DirectX::XMFLOAT3(0, 1, 0) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3(0, (float)i * size, NrOfBlocks * size), DirectX::XMFLOAT3(0, 1, 0) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3(NrOfBlocks * size, (float)i * size, 0), DirectX::XMFLOAT3(0, 1, 0) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3(NrOfBlocks * size, (float)i * size, NrOfBlocks * size), DirectX::XMFLOAT3(0, 1, 0) }));
        }

        //For X
        for (int i = 0; i < NrOfBlocks + 1; i++)
        {
            indecies.push_back((uint32_t)vertecies.size() + 0);
            indecies.push_back((uint32_t)vertecies.size() + 1);
            indecies.push_back((uint32_t)vertecies.size() + 2);
            indecies.push_back((uint32_t)vertecies.size() + 1);
            indecies.push_back((uint32_t)vertecies.size() + 3);
            indecies.push_back((uint32_t)vertecies.size() + 2);

            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3(0,                 0,                 (float)i * size), DirectX::XMFLOAT3(0, 0, 1) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3(NrOfBlocks * size, 0,                 (float)i * size), DirectX::XMFLOAT3(0, 0, 1) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3(0,                 NrOfBlocks * size, (float)i * size), DirectX::XMFLOAT3(0, 0, 1) }));
            vertecies.push_back(VoxelVertecies({ DirectX::XMFLOAT3(NrOfBlocks * size, NrOfBlocks * size, (float)i * size), DirectX::XMFLOAT3(0, 0, 1) }));
        }

        theReturnModel->subMeshes[0].vertexBuffer = createVertexBuffer(
            vertecies,
            gfx->getDevice(),
            gfx->getCommandList(),//unsure If I should I have a number here?
            gfx->getCommandAllocator(),//unsure If I should I have a number here?
            gfx->getCommandQueue(),
            gfx->getFenceEvent(),
            gfx->getFence(),
            gfx->getFenceValue()
        );

        theReturnModel->subMeshes[0].indeciesBuffer = createIndeciesBuffer(
            indecies,
            gfx->getDevice(),
            gfx->getCommandList(0),
            gfx->getCommandAllocator(0),
            gfx->getCommandQueue(),
            gfx->getFenceEvent(),
            gfx->getFence(),
            gfx->getFenceValue()
        );
        theReturnModel->subMeshes[0].nrOfIndecies = indecies.size();
        theReturnModel->subMeshes[0].vertexBufferView.BufferLocation = theReturnModel->subMeshes[0].vertexBuffer->GetGPUVirtualAddress();
        theReturnModel->subMeshes[0].vertexBufferView.SizeInBytes = vertecies.size() * sizeof(VoxelVertecies);
        theReturnModel->subMeshes[0].vertexBufferView.StrideInBytes = sizeof(VoxelVertecies);

        theReturnModel->subMeshes[0].indeciesBufferView.BufferLocation = theReturnModel->subMeshes[0].indeciesBuffer->GetGPUVirtualAddress();
        theReturnModel->subMeshes[0].indeciesBufferView.SizeInBytes = (UINT)(indecies.size() * sizeof(uint32_t));
        theReturnModel->subMeshes[0].indeciesBufferView.Format = DXGI_FORMAT_R32_UINT;

        return theReturnModel;
    }
}
