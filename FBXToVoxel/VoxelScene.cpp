#include "VoxelScene.h"
//#include "VoxelShaderClass.h"
#include "SpecialVoxelShader.h"
#include "ModelToVoxel.h"


VoxelScene::VoxelScene()
{
	
}

VoxelScene::~VoxelScene()
{
    for (int i = 0; i < nrOfLod; i++)
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
    std::vector<MaterialDescription> once = { MaterialDescription(), MaterialDescription() };

    shaderPtrForVoxel = new SpecialVoxelShader();
    ((SpecialVoxelShader*)shaderPtrForVoxel)->init(
        gfx->getDevice(),
        0,
        once,
        gfx->getInputLayout(2),
        "VoxelVertexShader.cso",
        "VoxelPixelShader.cso"
    );
    std::vector<MaterialDescription> one = { MaterialDescription() };
    shaderPtrForShadowVoxel = new SpecialVoxelShader();
    ((SpecialVoxelShader*)shaderPtrForShadowVoxel)->initS(
        gfx->getDevice(),
        0,
        one,
        gfx->getInputLayout(2),
        "VoxelVertexShader.cso",
        "PixelVoxelShadow.cso"
    );

    Light* testLight = lights->addLight(
        LightType::PointLight_E, DirectX::XMFLOAT3(0, 20, 0)
    );
    Light* testLight2 = lights->addLight(
        LightType::SpotLight_E, DirectX::XMFLOAT3(0, 20, 0)
    );
    testLight->setColor(DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f));
    testLight2->setShadowSoftNess(2);

    imguiHandler->addLight(lights->getLight(0));
    imguiHandler->addLight(lights->getLight(1));

    uint32_t size = chunkSize;
    for (uint32_t i = 0; i < nrOfLod; i++)
    {
        voxelModels[i] = GetVoxelModel(1 << i, chunkSize >> i);
    }

    //Model* Sponza = this->fileReader->readModel("../Models/Sponza.obj");
    //uint32_t SponzaObj = objectManager.createAnObject();
    //objectManager.getObject(SponzaObj)->addComponent(Sponza);
    //objectManager.getObject(SponzaObj)->setScale(DirectX::XMFLOAT3(0.3, 0.3, 0.3));

    Model* CameraModel = this->fileReader->readModel("../Models/Camera.fbx");
    LightObject = objectManager.createAnObject();
    objectManager.getObject(LightObject)->addComponent(CameraModel);
    LightObject2 = objectManager.createAnObject();
    objectManager.getObject(LightObject2)->addComponent(CameraModel);

    Model* PlaneModel = this->fileReader->readModel("../Models/Plane.fbx");
    uint32_t plane = objectManager.createAnObject();
    objectManager.getObject(plane)->addComponent(PlaneModel);
    objectManager.getObject(plane)->setPosition(DirectX::XMFLOAT3( -1, -1, -1));

    std::vector<uint32_t> shadowTex;
    shadowTex.push_back(1);
    DefaultMaterialData AAAA;
    AAAA.ka = DirectX::XMFLOAT4(1, 1, 1, 1);
    Material* ShadowMaterial = new Material(gfx, shadowTex, MaterialType::DIFFUSE_TEXTURE, sizeof(DefaultMaterialData), &AAAA);

    PlaneModel->subMeshes[0].material = ShadowMaterial;

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
    objectManager.getObject(LightObject)->setPosition(lights->getLight(0)->getPosition());

    objectManager.getObject(LightObject2)->setPosition(lights->getLight(1)->getPosition());
    objectManager.getObject(LightObject2)->setRotation(lights->getLight(1)->getRotation());

    if (keyboard->isKeyPressed('C')) {
        //camera.setPosition(lights->getLight(1)->getPosition());
        //camera.setRotation(lights->getLight(1)->getRotation());
        lights->getLight(1)->setPosition(camera.getPostion());
        lights->getLight(1)->setRotation(camera.getRotation());
    }
}

void VoxelScene::Render()
{
    //const std::vector<Object*> allObjects = objectManager.getAllObjects();
    //for (int i = 0; i < allObjects.size(); i++)
    //{
    //    renderer->render(allObjects[i]);
    //}
    //renderer->render(allObjects[0]);
    //VoxelPosition//SET DESCRIPTOR HEAP
    //shaderHandler->setShader(voxelShader);
    
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
    
    for (auto& itx : chunks) {
        for (auto& ity : itx.second) {
            for (auto& itz : ity.second) {
                const DirectX::XMFLOAT3 &chunkPos = itz.second->getPosition();
                const DirectX::XMFLOAT3 middleChunkPosition = DirectX::XMFLOAT3(chunkPos.x + (chunkSize / 2), chunkPos.y + (chunkSize / 2), chunkPos.z + (chunkSize / 2));
                //float distanceBetweenCameraAndChunkMiddle = HF::distance(this->camera.getPostion(), middleChunkPosition);
                float distanceBetweenCameraAndChunkMiddle = HF::distance(DirectX::XMFLOAT3(0,0,0), middleChunkPosition);

                //int lod = distanceBetweenCameraAndChunkMiddle / chunkSize;
                //lod = lod >= nrOfLod ? nrOfLod - 1 : lod;
                //
                int lod = 0;
                itz.second->setLOD(lod);

                CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(gfx->getTextureHeap().getHeap()->GetGPUDescriptorHandleForHeapStart());
                srvGpuHandle.Offset(itz.second->cbData.bindlessTextureIndex.x, descriptorSize);
                gfx->getCommandList()->SetGraphicsRootDescriptorTable(4, srvGpuHandle);
                
                itz.second->setConstantBuffers(gfx);//SET many times
                renderer->render(itz.second, voxelModels[itz.second->getLod()]);//Set many times
            }
        }
    }
}

void VoxelScene::RenderUI()
{
    if (ImGui::Begin("Start"))
    {
        if (ImGui::Button("Start"))
        {
            const float VoxelSize = 2;
            //load VoxelModel
            DirectX::XMUINT3 sizes;
            Voxel* voxelGrid = nullptr;
            ReadVoxelFromFile(sizes, voxelGrid, "../Models/VoxelTest.vox");

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
            
            for (int i = 0; i < nrOfChunks; i++)
            {
                TextureViewClass* voxelTextureData = createTexture(
                    (void*)convertedData[i],
                    sizeof(uint32_t),
                    DirectX::XMINT3(chunkSize, chunkSize, chunkSize),
                    rm,
                    gfx,
                    DXGI_FORMAT_R32_UINT
                );
                
                int chunkX = i % nrOfChunksInDirection.x;
                int chunkY = (i / nrOfChunksInDirection.x) % nrOfChunksInDirection.y;
                int chunkZ = i / (nrOfChunksInDirection.x * nrOfChunksInDirection.y);

                Chunk* theChunk = new Chunk(gfx);
                chunks[chunkX][chunkY][chunkZ] = theChunk;

                theChunk->setPosition(DirectX::XMFLOAT3(chunkX * chunkSize * VoxelSize, 
                                                        chunkY * chunkSize * VoxelSize, 
                                                        chunkZ * chunkSize * VoxelSize));
                theChunk->setScale(DirectX::XMFLOAT3(VoxelSize,
                                                     VoxelSize,
                                                     VoxelSize
                ));
                //theChunk->addComponent(voxelModels);

                static uint32_t nrOfChunks = 0;
                theChunk->cbData.bindlessTextureIndex.x = gfx->getTextureHeap().createSRV(nrOfChunks + MAXNROFLIGHTS, voxelTextureData, gfx->getDevice());
                nrOfChunks++;
                theChunk->updateConstantBuffers();
            }
            for (int i = 0; i < nrOfChunks; i++)
            {
                delete convertedData[i];
            }
            delete[] convertedData;
        }   
        if (ImGui::Button("Testing Box"))
        {   
            DirectX::XMUINT3 thesizes = { 200, 200, 200 };
            Voxel* voxelGrid = new Voxel[200 * 200 * 200];
            int i = 0;
            
            for (int x = 0; x < 200; x++) {
                for (int y = 0; y < 200; y++) {
                    for (int z = 0; z < 200; z++) {
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
            
            uint32_t* convertedData = new uint32_t[200 * 200 * 200];
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
            
                theChunk->cbData.bindlessTextureIndex.x = gfx->getTextureHeap().createSRV(voxelTextureData, gfx);
                theChunk->updateConstantBuffers();
            }
            
            delete[] convertedData;
        }
        if (ImGui::Button("VoxelCreator"))
        {
            sceneManager->setScene(new ModelToVoxel());
        }
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