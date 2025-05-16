#include "VoxelScene.h"
#include "SpecialVoxelShader.h"
#include "ModelToVoxel.h"
#include "TextureChanges.h"


VoxelScene::VoxelScene()
{
	
}

VoxelScene::~VoxelScene()
{
    delete shaderPtrForVoxel;
    delete shaderPtrForShadowVoxel;
    
    for (int i = 0; i < NROFLOD; i++)
    {
        delete voxelModels[i];
    }
    
    for (auto& itx : chunks) {
        for (auto& ity : itx.second) {
            for (auto& itz : ity.second) {
                delete itz.second;//Set many times
            }
        }
    }
}

void VoxelScene::Start()
{
    //change this some how
    //static_assert(chunkSize >= (8 << (NROFLOD - 1)), "chunkSize too small: Dispatch group size would be zero.");
    std::vector<MaterialDescription> once = { 
        MaterialDescription(1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV),
        MaterialDescription(1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
    };
    translationTextureHeapUAV.init(256, gfx->getDevice());

    shaderPtrForVoxel = new SpecialVoxelShader();
    ((SpecialVoxelShader*)shaderPtrForVoxel)->init(
        gfx->getDevice(),
        0,
        once,
        gfx->getInputLayout(2),
        "VoxelVertexShader.cso",
        "VoxelPixelShader.cso"
    );
    std::vector<MaterialDescription> one = { MaterialDescription({1})};
    shaderPtrForShadowVoxel = new SpecialVoxelShader();
    ((SpecialVoxelShader*)shaderPtrForShadowVoxel)->initS(
        gfx->getDevice(),
        0,
        one,
        gfx->getInputLayout(2),
        "VoxelVertexShader.cso",
        "PixelVoxelShadow.cso"
    );

    once[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    once[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    voxelMinimizerComputeShader = shaderHandler->createShader(0, once, "ComputeVoxelMinimizer.cso");

    Light* testLight2 = lights->addLight(
        LightType::SpotLight_E, DirectX::XMFLOAT3(0, 20, 0)
    );
    testLight2->setShadowSoftNess(1);

    imguiHandler->addLight(lights->getLight(1));

    uint32_t size = chunkSize;
    for (uint32_t i = 0; i < NROFLOD; i++)
    {
        voxelModels[i] = GetVoxelModel(1 << (i), chunkSize >> (i));
    }
}

void VoxelScene::Update(const float& dt)
{
    float speed = 20;
    if (keyboard->isKeyPressed(VK_CONTROL))
    {
        speed = 80;
    }
    else {
        speed = 20;
    }
    if (keyboard->isKeyPressed('A'))
    {
        camera.addPositionBasedOnDirection(DirectX::XMFLOAT3(1, 0, 0), speed * dt);
    }
    if (keyboard->isKeyPressed('W'))
    {
        camera.addPositionBasedOnDirection(DirectX::XMFLOAT3(0, 0, -1), speed * dt);
    }
    if (keyboard->isKeyPressed('D'))
    {
        camera.addPositionBasedOnDirection(DirectX::XMFLOAT3(-1, 0, 0), speed * dt);
    }
    if (keyboard->isKeyPressed('S'))
    {
        camera.addPositionBasedOnDirection(DirectX::XMFLOAT3(0, 0, 1), speed * dt);
    }
    if (keyboard->isKeyPressed(VK_SPACE))
    {
        camera.move(DirectX::XMFLOAT3(0, speed * dt, 0));
    }
    if (keyboard->isKeyPressed(VK_SHIFT))
    {
        camera.move(DirectX::XMFLOAT3(0, -speed * dt, 0));
    }

    camera.rotate(DirectX::XMFLOAT3(
        -(float)mouse->getDeltaPos().x * mouse->getSense(),
        (float)mouse->getDeltaPos().y * mouse->getSense(),
        0)
    );

    if (keyboard->isKeyPressed('C')) {
        lights->getLight(0)->setPosition(camera.getPostion());
        lights->getLight(0)->setRotation(camera.getRotation());
    }
    //Spin around
    if (keyboard->isKeyPressed('M'))
    {
        static float r = 0;
        static const float speed = 1;
        r += dt * speed;

        DirectX::XMFLOAT3 cameraPos = this->camera.getPostion();

        float x = spinAround.x + distanceFromMiddle * sinf(r);
        float z = spinAround.z + distanceFromMiddle * cosf(r);
        float y = spinAround.y + cameraHeight;

        // Update camera position
        camera.setPosition(DirectX::XMFLOAT3(x, y, z));

        this->camera.lookAt(spinAround);
    }
}

void VoxelScene::Render()
{

    //Sort Chunks; based on all certain camera
    std::vector<std::pair<float, Chunk*>> chunkWithDistace;
    for (const auto& x : chunks)
    {
        for (const auto& y : x.second)
        {
            for (const auto& z : y.second)
            {
                float magDistance = HF::magDistance(z.second->getPosition(), this->camera.getPostion());
                chunkWithDistace.emplace_back(magDistance, z.second);
            }
        }
    }
    std::sort(chunkWithDistace.begin(), chunkWithDistace.end(), [](const std::pair<float, Chunk*>& a, const std::pair<float, Chunk*>& b)
        {
            return a.first < b.first;
        });
    
    const UINT descriptorSize = gfx->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    shaderHandler->setShader(*(Shader*)shaderPtrForVoxel);
    shaderHandler->setShadowShaders(*(Shader*)shaderPtrForShadowVoxel);

    if (!renderer->isMakingShadows()) {
        const uint32_t nrOfShadowMaps = this->lights->getNrOfActiveShadowMaps();//(For default material);
        gfx->getCommandList()->SetGraphicsRootDescriptorTable(5, gfx->getTextureHeap().getHeap()->GetGPUDescriptorHandleForHeapStart());
    }
    else {
        lights->setCurrentLightAsPOV();
    }

    for (auto& it : chunkWithDistace)
    {
        const DirectX::XMFLOAT3& chunkPos = it.second->getPosition();
        const DirectX::XMFLOAT3 middleChunkPosition = DirectX::XMFLOAT3(chunkPos.x + (chunkSize / 2), chunkPos.y + (chunkSize / 2), chunkPos.z + (chunkSize / 2));
        float distanceBetweenCameraAndChunkMiddle = HF::magDistance(this->camera.getPostion(), middleChunkPosition);

        //LOD
        int lod = (int)log2(distanceBetweenCameraAndChunkMiddle / (chunkSize * chunkSize * 8));
        lod = std::clamp(lod, 0, NROFLOD - 1);
        it.second->setLOD(lod);


        CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(gfx->getTextureHeap().getHeap()->GetGPUDescriptorHandleForHeapStart());
        srvGpuHandle.Offset(it.second->cbData.bindlessTextureIndex.x, descriptorSize);
        gfx->getCommandList()->SetGraphicsRootDescriptorTable(4, srvGpuHandle);

        it.second->setConstantBuffers(gfx);//SET many times
        renderer->render(it.second, voxelModels[it.second->getLod()]);//Set many times
        
    }
}

void VoxelScene::RenderUI()
{
    if (ImGui::Begin("Start"))
    {
        if (ImGui::Button("Choose File"))
        {
            OpenFileDialog(InputFile);
        }
        if (ImGui::Button("Load Model"))
        { 
            //Delete old Model if it exist
            for (auto& x : chunks)
            {
                for (auto& y : x.second)
                {
                    for (auto& z : y.second)
                    {
                        delete z.second;
                    }
                    y.second.clear();
                }
                x.second.clear();
            }
            chunks.clear();
            translationTextureHeapUAV.reset();

            const float VoxelSize = 1;
            //load VoxelModel
            DirectX::XMUINT3 sizes;
            Voxel* voxelGrid = nullptr;
            ReadVoxelFromFile(sizes, voxelGrid, InputFile);

            if (voxelGrid == nullptr)
            {
                ImGui::End();
                return;
            }

            spinAround = DirectX::XMFLOAT3(
                (sizes.x / 2) * VoxelSize, 
                (sizes.y / 3) * VoxelSize,
                (sizes.z / 2) * VoxelSize);

            //convertData
            DirectX::XMUINT3 nrOfChunksInDirection;
            nrOfChunksInDirection.x = (sizes.x / chunkSize) + 1;
            nrOfChunksInDirection.y = (sizes.y / chunkSize) + 1;
            nrOfChunksInDirection.z = (sizes.z / chunkSize) + 1;

            uint32_t nrOfChunks = nrOfChunksInDirection.x * nrOfChunksInDirection.y * nrOfChunksInDirection.z;

            uint32_t** convertedData = new uint32_t * [nrOfChunks];
            for (uint32_t i = 0; i < nrOfChunks; i++)
            {
                convertedData[i] = new uint32_t[chunkSize * chunkSize * chunkSize]();
            }
            for (uint32_t z = 0; z < sizes.z; z++) {
                for (uint32_t y = 0; y < sizes.y; y++) {
                    for (uint32_t x = 0; x < sizes.x; x++) {
                        uint32_t chunkX = x / chunkSize;
                        uint32_t chunkY = y / chunkSize;
                        uint32_t chunkZ = z / chunkSize;
            
                        uint32_t chunkIndex =
                            chunkX +
                            chunkY * nrOfChunksInDirection.x +
                            chunkZ * nrOfChunksInDirection.x * nrOfChunksInDirection.y;
                        
                        uint32_t localX = x % chunkSize;
                        uint32_t localY = y % chunkSize;
                        uint32_t localZ = z % chunkSize;
            
                        uint32_t localIndex =
                            localX +
                            localY * chunkSize +
                            localZ * chunkSize * chunkSize;
                        
                        convertedData[chunkIndex][localIndex] =
                            (voxelGrid[z * sizes.x * sizes.y + y * sizes.x + x].rgb[0] << 16) |
                            (voxelGrid[z * sizes.x * sizes.y + y * sizes.x + x].rgb[1] << 8) |
                            (voxelGrid[z * sizes.x * sizes.y + y * sizes.x + x].rgb[2]);
            
                    }
                }
            }
            delete[] voxelGrid;

            const UINT descriptorSize = gfx->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            shaderHandler->setComputeShader(voxelMinimizerComputeShader);

            ID3D12DescriptorHeap* heaps[] = { translationTextureHeapUAV.getHeap() };
            gfx->getCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
            Chunk** theChunks = new Chunk*[nrOfChunks];
            TextureViewClass** voxelTextureData = new TextureViewClass*[NROFLOD * nrOfChunks];
            for (int c = 0; c < nrOfChunks; c++)
            {
                voxelTextureData[c * NROFLOD + 0] = createUAV(
                    (void*)convertedData[c],
                    sizeof(uint32_t),
                    DirectX::XMINT3(chunkSize, chunkSize, chunkSize),
                    rm,
                    gfx,
                    DXGI_FORMAT_R32_UINT
                );
                
                int chunkX = c % nrOfChunksInDirection.x;
                int chunkY = (c / nrOfChunksInDirection.x) % nrOfChunksInDirection.y;
                int chunkZ = c / (nrOfChunksInDirection.x * nrOfChunksInDirection.y);

                theChunks[c] = new Chunk(gfx);
                chunks[chunkX][chunkY][chunkZ] = theChunks[c];

                theChunks[c]->setPosition(DirectX::XMFLOAT3(chunkX * chunkSize * VoxelSize,
                                                        chunkY * chunkSize * VoxelSize, 
                                                        chunkZ * chunkSize * VoxelSize));
                theChunks[c]->setScale(DirectX::XMFLOAT3(VoxelSize,
                                                     VoxelSize,
                                                     VoxelSize
                ));

                uint32_t UAVS[NROFLOD];
                UAVS[0] = translationTextureHeapUAV.createUAV(voxelTextureData[c * NROFLOD + 0], gfx);
                for (int l = 1; l < NROFLOD; l++)
                {
                    DirectX::XMINT3 lodTextureSize(chunkSize / (l * 2), chunkSize / (l * 2), chunkSize / (l * 2));
                    voxelTextureData[c * NROFLOD + l] = createEmptyUAV(sizeof(uint32_t), lodTextureSize, gfx, DXGI_FORMAT_R32_UINT);

                    UAVS[l] = translationTextureHeapUAV.createUAV(voxelTextureData[c * NROFLOD + l], gfx);
                    
                    //Make a dispatch
                    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(translationTextureHeapUAV.getHeap()->GetGPUDescriptorHandleForHeapStart());
                    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle2 = srvGpuHandle;
                    srvGpuHandle.Offset(UAVS[l - 1], descriptorSize);
                    srvGpuHandle2.Offset(UAVS[l], descriptorSize);

                    gfx->getCommandList()->SetComputeRootDescriptorTable(0, srvGpuHandle);
                    gfx->getCommandList()->SetComputeRootDescriptorTable(1, srvGpuHandle2);

                    const uint32_t nrOfThreads = (chunkSize >> l);

                    gfx->getCommandList()->Dispatch(
                        nrOfThreads / 8, 
                        nrOfThreads / 8,  
                        nrOfThreads / 8
                    );
                }

                {
                    CD3DX12_RESOURCE_BARRIER toSRVBarrier[NROFLOD];
                    for (uint32_t l = 0; l < NROFLOD; l++)
                    {
                        toSRVBarrier[l] = CD3DX12_RESOURCE_BARRIER::Transition(
                            voxelTextureData[c * NROFLOD + l]->srvResource.Get(),
                            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE 
                        );
                    } 
                    gfx->getCommandList()->ResourceBarrier(NROFLOD, toSRVBarrier);
                }
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
            CheckHR(gfx->getCommandAllocator(0)->Reset())
            CheckHR(gfx->getCommandList(0)->Reset(gfx->getCommandAllocator(0), nullptr))
            int i = 0;
            for (uint32_t c = 0; c < nrOfChunks; c++)
            {
                for (uint32_t l = 0; l < NROFLOD; l++)
                {
                    theChunks[c]->setTexturePointerForLod(
                        gfx->getTextureHeap().createSRV(voxelTextureData[i++], gfx), 
                        l
                    );
                }
            }
            delete[] voxelTextureData;
            delete[] theChunks;
            for (int i = 0; i < nrOfChunks; i++)
            {
                delete convertedData[i];
            }
            delete[] convertedData;
            
        }   
        if (ImGui::Button("Testing Box"))
        {   
            DirectX::XMUINT3 thesizes = { chunkSize, chunkSize, chunkSize };
            Voxel* voxelGrid = new Voxel[(chunkSize  * chunkSize * chunkSize)];
            int i = 0;
            
            for (int x = 0; x < chunkSize; x++) {
                for (int y = 0; y < chunkSize; y++) {
                    for (int z = 0; z < chunkSize; z++) {
                        Voxel tempVoxel;
                        tempVoxel.rgb[0] = 0;
                        tempVoxel.rgb[1] = 0;
                        tempVoxel.rgb[2] = 0;
                        if (z % 3 == 0)
                        {
                            tempVoxel.rgb[0] = 244;
                            if (x % 2 == 0)
                            {
                                tempVoxel.rgb[1] = 254;
                            }
                            if (y % 2 == 0)
                            {
                                tempVoxel.rgb[2] = 254;
                            }
                            
                        }
                        voxelGrid[i] = tempVoxel;
                        i++;
                    }
                }
            }
            
            uint32_t* convertedData = new uint32_t[chunkSize * chunkSize * chunkSize];
            for (int z = 0; z < thesizes.z; z++) {
                for (int y = 0; y < thesizes.y; y++) {
                    for (int x = 0; x < thesizes.x; x++) {
            
            
                        uint32_t localX = x;
                        uint32_t localY = y;
                        uint32_t localZ = z;
            
                        uint32_t localIndex =
                            localX +
                            localY * chunkSize +
                            localZ * chunkSize * chunkSize;
            
                        convertedData[localIndex] =
                            (voxelGrid[z * thesizes.x * thesizes.y + y * thesizes.x + x].rgb[0] << 16) |
                            (voxelGrid[z * thesizes.x * thesizes.y + y * thesizes.x + x].rgb[1] << 8) |
                            (voxelGrid[z * thesizes.x * thesizes.y + y * thesizes.x + x].rgb[2]);
            
                    }
                }
            }
            
            int nrOfChunks = 1;
            for (int i = 0; i < nrOfChunks; i++)
            {
                TextureViewClass* voxelTextureData = createTexture(
                    (void*)convertedData,
                    sizeof(uint32_t),
                    DirectX::XMINT3(chunkSize, chunkSize, chunkSize),
                    rm,
                    gfx,
                    DXGI_FORMAT_R32_UINT
                );
            
                Chunk* theChunk = new Chunk(gfx);
                chunks[0][0][0] = theChunk;
            
                //theChunk->addComponent(voxelModels);
                theChunk->setPosition(DirectX::XMFLOAT3(-200,-200,-200));

                theChunk->setTexturePointerForLod(gfx->getTextureHeap().createSRV(voxelTextureData, gfx), 0);
                theChunk->updateConstantBuffers();
            }
            
            delete[] convertedData;
        }
        if (ImGui::Button("VoxelCreator"))
        {
            sceneManager->setScene(new ModelToVoxel());
        }

        ImGui::SliderFloat("DistanceFromMiddle", &distanceFromMiddle, 0, 400);
        ImGui::SliderFloat("CameraHeight", &cameraHeight, 0, 200);
    }
    
    ImGui::End();
}

Model* VoxelScene::GetVoxelModel(const int size, const int NrOfBlocks)
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