#include "ModelToVoxel.h"

#include "VoxelScene.h"
#include <unordered_map>
#include <unordered_set>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//STD::MAX/MIN WILL NOT WORK, FINE I DO IT MYSELF
#define max(x, y) x > y ? x : y
#define min(x, y) x < y ? x : y

uint32_t getIndex(const uint32_t x, const uint32_t y, const uint32_t z, const uint32_t width, const uint32_t height)
{
    return x + (y * width) + (z * width * height);
}

DirectX::XMFLOAT3 add(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
    return DirectX::XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}
DirectX::XMFLOAT3 sub(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
{
    return DirectX::XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}
DirectX::XMFLOAT3 mul(const DirectX::XMFLOAT3& a, const float& b)
{
    return DirectX::XMFLOAT3(a.x * b, a.y * b, a.z * b);
}
DirectX::XMFLOAT3 div(const DirectX::XMFLOAT3& a, const float& b)
{
    return DirectX::XMFLOAT3(a.x / b, a.y / b, a.z / b);
}
DirectX::XMINT3 F3ToI3(const DirectX::XMFLOAT3& a)
{
    return DirectX::XMINT3(a.x, a.y, a.z);
}

ModelToVoxel::ModelToVoxel()
{
}

ModelToVoxel::~ModelToVoxel()
{
}

void ModelToVoxel::Start()
{
    objectManager.createAnObject();


    //uint32_t data = 5;
    //rbBuffer = new ReadBackBuffer(
    //    &data, 
    //    sizeof(uint32_t),
    //    gfx
    //);
    std::vector<MaterialDescription> ForComputeVoxels;
    ForComputeVoxels.push_back(MaterialDescription(1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV));//VoxelGrid
    ForComputeVoxels.push_back(MaterialDescription(1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV));//Vertecies
    ForComputeVoxels.push_back(MaterialDescription(1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV));//Indecies
    ForComputeVoxels.push_back(MaterialDescription(1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV));//WhatTexture
    ForComputeVoxels.push_back(MaterialDescription(200, D3D12_DESCRIPTOR_RANGE_TYPE_SRV));//MeshTexture

    computeVoxelsShader = shaderHandler->createShader(1, ForComputeVoxels, "ComputeVoxels.cso");
    rbBufferHeap.init(300, gfx->getDevice());
    
    //rbBufferHeap.createUAV(rbBuffer->getUAVResource(), gfx->getDevice());

    //getBox();
}

void ModelToVoxel::Update(const float& dt)
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
}

void ModelToVoxel::Render()
{
    //if (doneShit)
    //{
    //    uint32_t* data = this->rbBuffer->getData<uint32_t>(gfx);
    //}
}

void OpenFileDialog(std::string& fileName) {
    wchar_t  filename[MAX_PATH] = { 0 };

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.TXT\0";
    ofn.lpstrFile = filename;  // Now it's correctly using a wchar_t buffer
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        std::wstring wFilename(filename);
        fileName = std::string(wFilename.begin(), wFilename.end());
    }
}

void ModelToVoxel::RenderUI()
{
    static bool nextGo = false;
    if (nextGo)
    {
        information = "";
        nextGo = false;
        Voxel* voxels = CreateVoxelModelGPU();
        bool isnull = true;
        for (uint32_t i = 0; i < sizes.x * sizes.y * sizes.z && isnull; i++)
        {
            if (voxels[i].rgb[0] + voxels[i].rgb[1] + voxels[i].rgb[2] != 0)
            {
                isnull = false;
            }
        }

        if (isnull)
        {
            delete[] voxels;
            return;
        }

        if (voxels == nullptr)
        {
            return;
        }
        WriteVoxelToFile(
            sizes,
            voxels,
            "VoxelTest.vox"
        );
        delete[] voxels;
        information += "\nSucessfully created VoxelModel";
    }

    if (ImGui::Begin("Voxel"))
    {

        ImGui::InputInt3("Voxel Grid Size", (int*)&sizes);
        if (ImGui::Button("Open File Dialog"))
        {
            OpenFileDialog(this->fileName);
        }
        if (ImGui::Button("Start Creating VoxelModel GPU"))
        {
            nextGo = true;
        }
        if (ImGui::Button("Start Creating VoxelModel CPU"))
        {   
            information = "";
            {
                Voxel* voxels = this->CreateVoxelModelCPU();
                if (voxels == nullptr)
                {
                    return;
                }
                WriteVoxelToFile(
                    sizes,
                    voxels,
                    "VoxelTest.vox"
                );
                delete[] voxels;
                information += "\nSucessfully created VoxelModel";
            }
            information += "\nSucessfully created VoxelModel";
        }
        if (ImGui::Button("Go To VoxelScene"))
        {
            sceneManager->setScene(new VoxelScene());
        }
        {
            std::string fileInfo = "Choosen File : " + fileName;
            if (fileName == "")
            {
                fileInfo = "No File Choosen";
            }
            ImGui::Text(fileInfo.c_str());
        }
        if (information != "")
        {
            ImGui::Text(information.c_str());
        }
    }

    

    ImGui::End();
}

Voxel* ModelToVoxel::CreateVoxelModelGPU()
{
    VoxelModel model;

    
    LoadModelForGPU(model, rm);

    auto _TimeStart = std::chrono::steady_clock::now();

    if (model.meshes.size() == 0)
    {
        return nullptr;
    }

    {
        float xSize = boundingBox[1].x - boundingBox[0].x;
        float ySize = boundingBox[1].y - boundingBox[0].y;
        float zSize = boundingBox[1].z - boundingBox[0].z;

        float scaleFactor = min(min(sizes.x / xSize, sizes.y / ySize), sizes.z / zSize);
        sizes.x = std::floor((scaleFactor + 0.00001) * xSize);
        sizes.y = std::floor((scaleFactor + 0.00001) * ySize);
        sizes.z = std::floor((scaleFactor + 0.00001) * zSize);
    }

    VoxelGPU* voxelGrid = new VoxelGPU[sizes.x * sizes.y * sizes.z];

    float voxelSize = (boundingBox[1].x - boundingBox[0].x) / (sizes.x - 1);
    voxelSize = max(voxelSize, (boundingBox[1].y - boundingBox[0].y) / (sizes.y - 1));
    voxelSize = max(voxelSize, (boundingBox[1].z - boundingBox[0].z) / (sizes.z - 1));
    voxelSize += 0.0000001f;//BIAS IS NEEDED
    const DirectX::XMFLOAT3 minSizes = boundingBox[0];

    creatingVoxelModelDataData.sizes = DirectX::XMUINT4(sizes.x, sizes.y, sizes.z, 0);
    creatingVoxelModelDataData.minSizes = DirectX::XMFLOAT4(minSizes.x, minSizes.y, minSizes.z, 0);
    creatingVoxelModelDataData.voxelSize.x = voxelSize;
    creatingVoxelModelDataData.voxelSize.w = model.IndeciesStartAndEnd.size();

    creatingVoxelModelData = CreateConstantBuffer(gfx, creatingVoxelModelDataData);

    //ADD VOXELS HERE
    rbBuffer = new ReadBackBuffer(
        voxelGrid,
        sizeof(VoxelGPU) * sizes.x * sizes.y * sizes.z,
        gfx
    );

    rbBuffer->positionInHeap = rbBufferHeap.createUAV(
        rbBuffer->getUAVResource(), gfx->getDevice(), sizes.x * sizes.y * sizes.z, sizeof(VoxelGPU)
    );


    //Get first good texture
    for (uint32_t i = 0; i < model.IndeciesStartAndEnd.size(); i++)
    {
        model.IndeciesStartAndEnd[i].w = -1;
    }
    int32_t firstGoodTexture = -1;

    const uint32_t startOfTextures = 10;
    uint32_t position = 0;
    
    for (uint32_t i = 0; i < model.IndeciesStartAndEnd.size(); i++)
    {
        if (model.texturesGPU[model.IndeciesStartAndEnd[i].z] != nullptr)
        {
            model.IndeciesStartAndEnd[i].w = rbBufferHeap.createSRV(
                position + startOfTextures, model.texturesGPU[model.IndeciesStartAndEnd[i].z], gfx->getDevice()
            ) - startOfTextures;
            if (model.IndeciesStartAndEnd[i].w == position)
            {
                position++;
            }
            if (firstGoodTexture == -1)
            {
                firstGoodTexture = model.IndeciesStartAndEnd[i].w;
            }
        }
    }
    for (uint32_t i = 0; i < model.IndeciesStartAndEnd.size(); i++)
    {
        if (model.IndeciesStartAndEnd[i].w == -1)
        {
            model.IndeciesStartAndEnd[i].w = firstGoodTexture;
        }
    }
    

    for (int m = 0; m < model.meshes.size(); m++)
    {
        creatingVoxelModelDataData.sizes.w = model.meshes[m].indecies.size();
        updateConstantBuffer(creatingVoxelModelDataData, creatingVoxelModelData);

        shaderHandler->setComputeShader(this->computeVoxelsShader);
        ID3D12DescriptorHeap* heaps[] = { rbBufferHeap.getHeap() };
        gfx->getCommandList(0)->SetDescriptorHeaps(_countof(heaps), heaps);

        GraphicsBufferWithData computeVoxelData[3];
        computeVoxelData[0].init(
            model.meshes[m].vertecies.data(),
            sizeof(Vertecies)* model.meshes[m].vertecies.size(),
            gfx
        );
        computeVoxelData[0].posInHeap = rbBufferHeap.createNormalResource(
            computeVoxelData[0].resource,
            gfx->getDevice(),
            sizeof(Vertecies),
            model.meshes[m].vertecies.size()
        );

        computeVoxelData[1].init(
            model.meshes[m].indecies.data(),
            sizeof(uint32_t)* model.meshes[m].indecies.size(),
            gfx
        );
        computeVoxelData[1].posInHeap = rbBufferHeap.createNormalResource(
            computeVoxelData[1].resource,
            gfx->getDevice(),
            sizeof(uint32_t),
            model.meshes[m].indecies.size()
        );

        computeVoxelData[2].init(
            model.IndeciesStartAndEnd.data(),
            sizeof(DirectX::XMUINT4) * model.IndeciesStartAndEnd.size(),
            gfx
        );
        computeVoxelData[2].posInHeap = rbBufferHeap.createNormalResource(
            computeVoxelData[2].resource,
            gfx->getDevice(),
            sizeof(DirectX::XMUINT4),
            model.IndeciesStartAndEnd.size()
        );
        

        //updateConstantBuffer(creatingVoxelModelDataData, creatingVoxelModelData);

        //SET Data for voxels
        gfx->getCommandList(0)->SetComputeRootConstantBufferView(0, creatingVoxelModelData.constantBuffer->GetGPUVirtualAddress());

        const UINT descriptorSize = gfx->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(rbBufferHeap.getHeap()->GetGPUDescriptorHandleForHeapStart());
        CD3DX12_GPU_DESCRIPTOR_HANDLE ChangeingHandle = srvGpuHandle;

        //Voxels
        ChangeingHandle.Offset(rbBuffer->positionInHeap, descriptorSize);
        gfx->getCommandList(0)->SetComputeRootDescriptorTable(1, ChangeingHandle);
        ChangeingHandle = srvGpuHandle;
        //Vertecies
        ChangeingHandle.Offset(computeVoxelData[0].posInHeap, descriptorSize);
        gfx->getCommandList(0)->SetComputeRootDescriptorTable(2, ChangeingHandle);
        ChangeingHandle = srvGpuHandle;
        //Indecies
        ChangeingHandle.Offset(computeVoxelData[1].posInHeap, descriptorSize);
        gfx->getCommandList(0)->SetComputeRootDescriptorTable(3, ChangeingHandle);
        ChangeingHandle = srvGpuHandle;
        //WhatTexture
        ChangeingHandle.Offset(computeVoxelData[2].posInHeap, descriptorSize);
        gfx->getCommandList(0)->SetComputeRootDescriptorTable(4, ChangeingHandle);
        ChangeingHandle = srvGpuHandle;
        //The Textures
        ChangeingHandle.Offset(startOfTextures, descriptorSize);
        gfx->getCommandList(0)->SetComputeRootDescriptorTable(5, ChangeingHandle);

        //THIS RIGHT NOW MAKES IT ONE THREAD BUT JUST TO TEST
        const uint32_t triangleCount = model.meshes[m].indecies.size() / 3;
        const uint32_t threadGroupSize = 64;
        const uint32_t dispatchX = (triangleCount + threadGroupSize - 1) / threadGroupSize;
        gfx->getCommandList(0)->Dispatch(
            dispatchX,
            1, 
            1
        );

        {
            CheckHR(gfx->getCommandList(0)->Close())
            ID3D12CommandList* const commandLists[] = { gfx->getCommandList(0) };
            gfx->getCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);
        }
        CheckHR(gfx->getCommandQueue()->Signal(gfx->getFence(), ++gfx->getFenceValue()))
        CheckHR(gfx->getFence()->SetEventOnCompletion(gfx->getFenceValue(), gfx->getFenceEvent()))
        if (::WaitForSingleObject(gfx->getFenceEvent(), 20000) == WAIT_FAILED)
        {
            breakDebug;
        }
        CheckHR(gfx->getCommandAllocator(0)->Reset())
        CheckHR(gfx->getCommandList(0)->Reset(gfx->getCommandAllocator(0), nullptr))

        //DELETE AND DO EVERYTHING AGAIN
        rbBufferHeap.removeFromHeap(computeVoxelData[0].posInHeap);
        rbBufferHeap.removeFromHeap(computeVoxelData[1].posInHeap);
        rbBufferHeap.removeFromHeap(computeVoxelData[2].posInHeap);

        computeVoxelData[0].reset();
        computeVoxelData[1].reset();
        computeVoxelData[2].reset();

        doneShit = true;
    }
    voxelGrid = rbBuffer->getData<VoxelGPU>(gfx);
    //Make the VoxelGPU(This uses uint32_t) to Voxel(This uses uint16_t)

    Voxel* returnVoxelGrid = new Voxel[sizes.x * sizes.y * sizes.z];

    for (uint32_t i = 0; i < sizes.x * sizes.y * sizes.z; i++)
    {
        returnVoxelGrid[i].rgb[0] = voxelGrid[i].rgb[2];
        returnVoxelGrid[i].rgb[1] = voxelGrid[i].rgb[1];
        returnVoxelGrid[i].rgb[2] = voxelGrid[i].rgb[0];
    }

    delete[] voxelGrid;

    auto _TimeEnd = std::chrono::steady_clock::now();

    std::chrono::duration<float> duration = _TimeEnd - _TimeStart;
    float TotalTime = duration.count();

    information += "\nTook " + std::to_string(TotalTime) + " Seconds to create VoxelModel on GPU";

    return returnVoxelGrid;
}

void ModelToVoxel::LoadModelForGPU(
    VoxelModel& theReturn,
    ResourceManager* rm
)
{
    struct stat buffer;
    if (!(stat(this->fileName.c_str(), &buffer) == 0)) {
        return;
    }

    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(fileName.c_str(),
        aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs
    );

    if (!pScene) {
        std::wcout << L"couldn't find " << fileName.c_str() << " in directory: " << HF::getCurrentDirectory() << std::endl;
        printf("Error");
        return;
    }

    theReturn.meshes.resize(1);

    boundingBox[0] = DirectX::XMFLOAT3(D3D12_FLOAT32_MAX, D3D12_FLOAT32_MAX, D3D12_FLOAT32_MAX);
    boundingBox[1] = DirectX::XMFLOAT3(-D3D12_FLOAT32_MAX, -D3D12_FLOAT32_MAX, -D3D12_FLOAT32_MAX);

    //LOADING MATERIALS
    theReturn.texturesGPU.resize(pScene->mNumMaterials);
    for (uint32_t m = 0; m < pScene->mNumMaterials; m++)
    {
        const aiMaterial* pMaterial = pScene->mMaterials[m];
        aiString path;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
        {
            std::string pathString = "";
            
            if (path.C_Str()[0] != 'C' && path.C_Str()[1] != ':')
            {
                pathString = "../";
            }
            
            pathString += path.C_Str();
    
            theReturn.texturesGPU[m] = rm->getResource<TextureViewClass>("VoxelTextures__" + pathString);
            if (theReturn.texturesGPU[m] == nullptr)
            {
                theReturn.texturesGPU[m] = createTexture(pathString, rm, gfx, 1);
                //theReturn.texturesGPU[m] = createTextureWithWriteAccess(pathString, rm, gfx, 1);
                rm->addResource(theReturn.texturesGPU[m], "VoxelTextures__" + pathString);
            }
        }
    }

    uint32_t vertexOffset = 0;
    //LOAD MESHES
    for (uint32_t m = 0; m < pScene->mNumMeshes; m++)
    {
        aiMesh* mesh = pScene->mMeshes[m];
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertecies vertex;
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;
            if (mesh->mTextureCoords[0] != nullptr)
            {
                vertex.uv.x = mesh->mTextureCoords[0][i].x;
                vertex.uv.y = mesh->mTextureCoords[0][i].y;
            }
            boundingBox[0].x = min(vertex.position.x, boundingBox[0].x);
            boundingBox[0].y = min(vertex.position.y, boundingBox[0].y);
            boundingBox[0].z = min(vertex.position.z, boundingBox[0].z);
            boundingBox[1].x = max(vertex.position.x, boundingBox[1].x);
            boundingBox[1].y = max(vertex.position.y, boundingBox[1].y);
            boundingBox[1].z = max(vertex.position.z, boundingBox[1].z);

            theReturn.meshes[0].vertecies.push_back(vertex);
        }

        theReturn.IndeciesStartAndEnd.push_back(DirectX::XMINT4(
            theReturn.meshes[0].indecies.size(),
            theReturn.meshes[0].indecies.size() + (mesh->mNumFaces * 3),
            mesh->mMaterialIndex,
            0
        ));

        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            theReturn.meshes[0].indecies.push_back(mesh->mFaces[i].mIndices[0] + vertexOffset);
            theReturn.meshes[0].indecies.push_back(mesh->mFaces[i].mIndices[1] + vertexOffset);
            theReturn.meshes[0].indecies.push_back(mesh->mFaces[i].mIndices[2] + vertexOffset);
        }
        
        //TODO: Take care of soon
        //theReturn.meshes[m].materialIndex = mesh->mMaterialIndex;
        
        vertexOffset += mesh->mNumVertices;
    }
}

void ModelToVoxel::LoadModelForCPU(VoxelModel& theReturn, ResourceManager* rm)
{
    struct stat buffer;
    if (!(stat(this->fileName.c_str(), &buffer) == 0)) {
        information = "Error : Can't find model";
        return;
    }
    information += "\n reading from assimp";
    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(fileName.c_str(),
        aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs
    );

    if (!pScene) {
        information += "Error Couldn't find file";
        std::wcout << L"couldn't find " << fileName.c_str() << " in directory: " << HF::getCurrentDirectory() << std::endl;
        printf("Error");
        return;
    }
    information += "Loading vertecies & indecies";
    theReturn.meshes.resize(pScene->mNumMeshes);

    boundingBox[0] = DirectX::XMFLOAT3(D3D12_FLOAT32_MAX, D3D12_FLOAT32_MAX, D3D12_FLOAT32_MAX);
    boundingBox[1] = DirectX::XMFLOAT3(-D3D12_FLOAT32_MAX, -D3D12_FLOAT32_MAX, -D3D12_FLOAT32_MAX);

    //LOADING MATERIALS
    theReturn.texturesCPU.resize(pScene->mNumMaterials);
    for (uint32_t m = 0; m < pScene->mNumMaterials; m++)
    {
        const aiMaterial* pMaterial = pScene->mMaterials[m];
        aiString path;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
        {
            std::string pathString = "";

            if (path.C_Str()[0] != 'C' && path.C_Str()[1] != ':')
            {
                pathString = "../";
            }

            pathString += path.C_Str();

            theReturn.texturesCPU[m] = rm->getResource<TextureForVoxels>("VoxelTextures__" + pathString);
            if (theReturn.texturesCPU[m] == nullptr)
            {
                theReturn.texturesCPU[m] = LoadTexture(pathString);
                rm->addResource(theReturn.texturesCPU[m], "VoxelTextures__" + pathString);
            }
        }
    }

    //LOAD MESHES
    for (uint32_t m = 0; m < pScene->mNumMeshes; m++)
    {
        aiMesh* mesh = pScene->mMeshes[m];
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertecies vertex;
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;
            if (mesh->mTextureCoords[0] != nullptr)
            {
                vertex.uv.x = mesh->mTextureCoords[0][i].x;
                vertex.uv.y = mesh->mTextureCoords[0][i].y;
            }
            boundingBox[0].x = min(vertex.position.x, boundingBox[0].x);
            boundingBox[0].y = min(vertex.position.y, boundingBox[0].y);
            boundingBox[0].z = min(vertex.position.z, boundingBox[0].z);
            boundingBox[1].x = max(vertex.position.x, boundingBox[1].x);
            boundingBox[1].y = max(vertex.position.y, boundingBox[1].y);
            boundingBox[1].z = max(vertex.position.z, boundingBox[1].z);

            theReturn.meshes[m].vertecies.push_back(vertex);
        }
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            theReturn.meshes[m].indecies.push_back(mesh->mFaces[i].mIndices[0]);
            theReturn.meshes[m].indecies.push_back(mesh->mFaces[i].mIndices[1]);
            theReturn.meshes[m].indecies.push_back(mesh->mFaces[i].mIndices[2]);
        }
        theReturn.meshes[m].materialIndex = mesh->mMaterialIndex;
    }
}

void ModelToVoxel::TriangleLoading(
    const VoxelModel& model,
    const uint32_t start,
    const uint32_t offset,
    const uint32_t m,
    const float voxelSize, 
    const DirectX::XMFLOAT3& minSizes,
    Voxel* voxelGrid
)
{
    for (int i = start * 3; i < model.meshes[m].indecies.size(); i += 3 * offset)
    {
        //FOR EACH TRIANGLE
        //Localize Vertex Points
        DirectX::XMINT3 voxelPosition[3];
        const Vertecies* vertecies[3] = {
            &model.meshes[m].vertecies[model.meshes[m].indecies[i + 0]],
            &model.meshes[m].vertecies[model.meshes[m].indecies[i + 1]],
            &model.meshes[m].vertecies[model.meshes[m].indecies[i + 2]]
        };
        uint32_t low = UINT32_MAX, high = 0;//HEIGHT BOUNDING LINE

        for (uint8_t j = 0; j < 3; j++)
        {
            const DirectX::XMFLOAT3 vertexPos = vertecies[j]->position;

            //DO THIS WITH VECTORS INSTEAD OF XMINT3
            DirectX::XMFLOAT3 positionInGrid = div(sub(vertexPos, minSizes), voxelSize);
            voxelPosition[j] = F3ToI3(positionInGrid);
        }

        //unordered_map<Y, std::vector<<X,Z>, UV>>
        std::unordered_map<int, std::vector<std::pair<DirectX::XMINT2, DirectX::XMFLOAT2>>> BlocksForLineDrawing;
        const TextureForVoxels* CurrentTexture = model.texturesCPU[model.meshes[m].materialIndex];
        //Create lines from each vertex
        for (int j = 0; j < 3; j++)
        {
            const DirectX::XMINT3 endVoxel = voxelPosition[(j + 1) % 3];
            const DirectX::XMINT3 startVoxel = voxelPosition[j];

            DirectX::XMINT3 distance = DirectX::XMINT3(
                endVoxel.x - startVoxel.x,
                endVoxel.y - startVoxel.y,
                endVoxel.z - startVoxel.z
            );

            DirectX::XMINT3 stepDirection = DirectX::XMINT3(
                (distance.x > 0) ? 1 : distance.x < 0 ? -1 : 0,
                (distance.y > 0) ? 1 : distance.y < 0 ? -1 : 0,
                (distance.z > 0) ? 1 : distance.z < 0 ? -1 : 0
            );

            float l = sqrt(distance.x * distance.x + distance.y * distance.y + distance.z * distance.z);
            DirectX::XMFLOAT3 lineDirection = DirectX::XMFLOAT3(
                distance.x / l,
                distance.y / l,
                distance.z / l
            );

            DirectX::XMFLOAT3 stepLength = DirectX::XMFLOAT3(
                stepDirection.x != 0 ? std::abs(1.0f / lineDirection.x) : D3D12_FLOAT32_MAX,
                stepDirection.y != 0 ? std::abs(1.0f / lineDirection.y) : D3D12_FLOAT32_MAX,
                stepDirection.z != 0 ? std::abs(1.0f / lineDirection.z) : D3D12_FLOAT32_MAX
            );

            DirectX::XMFLOAT3 tMax = DirectX::XMFLOAT3(
                0,
                0,
                0
            );
            DirectX::XMINT3 traverseVoxel = startVoxel;

            while (memcmp(&traverseVoxel, &endVoxel, sizeof(DirectX::XMINT3)) != 0)//THIS IS CORRECT
            {
                if (tMax.x < tMax.y && tMax.x < tMax.z) {
                    traverseVoxel.x += stepDirection.x;
                    tMax.x += stepLength.x;
                }
                else if (tMax.y < tMax.z) {
                    traverseVoxel.y += stepDirection.y;
                    tMax.y += stepLength.y;
                }
                else {
                    traverseVoxel.z += stepDirection.z;
                    tMax.z += stepLength.z;
                }

                //
                Vertecies startVertex = *vertecies[j];
                Vertecies endVertex = *vertecies[(j + 1) % 3];

                float procent = HF::distance(traverseVoxel, startVoxel) / HF::distance(startVoxel, endVoxel);
                DirectX::XMFLOAT2 uv(
                    startVertex.uv.x + (endVertex.uv.x - startVertex.uv.x) * procent,
                    startVertex.uv.y + (endVertex.uv.y - startVertex.uv.y) * procent
                );
                const DirectX::XMUINT4 rgb = ColorFromUVAndTexture(uv, CurrentTexture);

                uint32_t index = getIndex(traverseVoxel.x, traverseVoxel.y, traverseVoxel.z >= sizes.z ? sizes.z - 1 : traverseVoxel.z, sizes.x, sizes.y);

                if (rgb.w == 0)
                {
                    continue;
                }

                voxelGrid[index].rgb[0] = rgb.x;
                voxelGrid[index].rgb[1] = rgb.y;
                voxelGrid[index].rgb[2] = rgb.z;

                if (rgb.x + rgb.y + rgb.z == 0)//If it's black just add one bcs 0 should be nothing
                {
                    voxelGrid[index].rgb[0] = 1;
                }

                if (j == 2)
                {
                    const DirectX::XMINT3 MidPoint = voxelPosition[(j + 2) % 3];
                    //Do a line shit
                    lineToLine(
                        MidPoint,
                        traverseVoxel,
                        uv,
                        vertecies[(j + 2) % 3]->uv,
                        CurrentTexture,
                        voxelGrid
                    );
                }
            }
        }
    }
}

TextureForVoxels* ModelToVoxel::LoadTexture(const std::string& filePath)
{
    struct stat buffer;
    if (!(stat(filePath.c_str(), &buffer) == 0)) {
        std::cout << "Error couldn't find texture" << std::endl;
        return nullptr;
    }
    TextureForVoxels* theReturn = new TextureForVoxels();
    
    int comp;
    unsigned char* data = stbi_load(filePath.c_str(), (int*)&theReturn->width, (int*)&theReturn->height, &comp, 0);
    
    theReturn->uvIncrement.x = 1.0f / theReturn->width;
    theReturn->uvIncrement.y = 1.0f / theReturn->height;
    theReturn->data = new uint32_t[theReturn->width * theReturn->height];
    for (uint32_t i = 0; i < theReturn->width * theReturn->height; i++)
    {
        uint32_t r = data[i * comp + 0];
        uint32_t g = data[i * comp + 1];
        uint32_t b = data[i * comp + 2];
        uint32_t a = 255;
        if (comp == 4)
        {
            a = data[i * comp + 3];
        }

        theReturn->data[i] = (a << 24) | (r << 16) | (g << 8) | (b << 0);//TODO : CHECK IF IT is it the reverse?
    }

    stbi_image_free(data);

    return theReturn;
}

DirectX::XMUINT4 ModelToVoxel::ColorFromUVAndTexture(DirectX::XMFLOAT2 UV, const TextureForVoxels* texture)
{
    if (texture == nullptr || texture->width == 0)
    {
        return DirectX::XMUINT4(0, 0, 0, 0);
    }
    //TODO : don't know if it should be fliped
    //In case of uv.x|y > 1
    UV.x -= floor(UV.x);
    UV.y -= floor(UV.y);
    //UV.y = 1 - UV.y;
    //UV.x = 1 - UV.x;
        
    int texPosX = UV.x * (texture->width);
    int texPosY = UV.y * (texture->height);
    texPosX = min(texPosX, texture->width - 1);
    texPosY = min(texPosY, texture->height - 1);

    uint32_t packedColor = texture->data[texPosY * texture->width + texPosX];

    uint8_t r = (packedColor >> 0) & 0xFF;
    uint8_t g = (packedColor >> 8) & 0xFF;
    uint8_t b = (packedColor >> 16) & 0xFF;
    uint8_t a = (packedColor >> 24) & 0xFF;

    return DirectX::XMUINT4(r, g, b, a);
}

Voxel* ModelToVoxel::CreateVoxelModelCPU() {
    
    VoxelModel model;

    
    LoadModelForCPU(model, rm);
    auto _TimeStart = std::chrono::steady_clock::now();

    if (model.meshes.size() == 0)
    {
        return nullptr;
    }

    //Create a smaller version
    {
        float xSize = boundingBox[1].x - boundingBox[0].x;
        float ySize = boundingBox[1].y - boundingBox[0].y;
        float zSize = boundingBox[1].z - boundingBox[0].z;

        float scaleFactor = min(min(sizes.x / xSize, sizes.y / ySize), sizes.z / zSize );
        sizes.x = std::floor((scaleFactor + 0.00001) * xSize);
        sizes.y = std::floor((scaleFactor + 0.00001) * ySize);
        sizes.z = std::floor((scaleFactor + 0.00001) * zSize);
    }

    Voxel* voxelGrid = new Voxel[sizes.x * sizes.y * sizes.z];

    float voxelSize = (boundingBox[1].x - boundingBox[0].x) / (sizes.x - 1);
    voxelSize = max(voxelSize, (boundingBox[1].y - boundingBox[0].y) / (sizes.y - 1));
    voxelSize = max(voxelSize, (boundingBox[1].z - boundingBox[0].z) / (sizes.z - 1));
    voxelSize += 0.0000001f;//BIAS IS NEEDED
    const DirectX::XMFLOAT3 minSizes = boundingBox[0];

    //const int nrOfThreads = std::thread::hardware_concurrency() - 2;
    const int nrOfThreads = model.meshes.size();
    //information += "\nnrOfThreads " + std::to_string(nrOfThreads) + "\n";
    //const int nrOfThreads = 1;
    std::thread* threads = new std::thread[nrOfThreads];

    for (uint32_t m = 0; m < model.meshes.size(); m++) 
    {
        threads[m] = std::thread(&ModelToVoxel::TriangleLoading, this,
            model,
            0,
            1,
            m,
            voxelSize,
            minSizes,
            voxelGrid
        );
    }
    for (int i = 0; i < nrOfThreads; i++)
    {
        threads[i].join();
    }

    auto _TimeEnd = std::chrono::steady_clock::now();

    std::chrono::duration<float> duration = _TimeEnd - _TimeStart;
    float TotalTime = duration.count();

    information += "CPU time took " + std::to_string(TotalTime) + " Time";

    return voxelGrid;
}

//TODO : DO WITH VERTEX AND VOXEL POSITION
void ModelToVoxel::lineToLine(
    const DirectX::XMINT3& startVoxel,
    const DirectX::XMINT3& endVoxel,
    const DirectX::XMFLOAT2& startUV, 
    const DirectX::XMFLOAT2& endUV,
    const TextureForVoxels* Texture,
    Voxel* voxelGrid
)
{
    DirectX::XMINT3 distance = DirectX::XMINT3(
        endVoxel.x - startVoxel.x,
        endVoxel.y - startVoxel.y,
        endVoxel.z - startVoxel.z
    );

    DirectX::XMINT3 stepDirection = DirectX::XMINT3(
        (distance.x > 0) ? 1 : distance.x < 0 ? -1 : 0,
        (distance.y > 0) ? 1 : distance.y < 0 ? -1 : 0,
        (distance.z > 0) ? 1 : distance.z < 0 ? -1 : 0
    );

    float l = sqrt(distance.x * distance.x + distance.y * distance.y + distance.z * distance.z);
    DirectX::XMFLOAT3 lineDirection = DirectX::XMFLOAT3(
        distance.x / l,
        distance.y / l,
        distance.z / l
    );

    DirectX::XMFLOAT3 stepLength = DirectX::XMFLOAT3(
        stepDirection.x != 0 ? std::abs(1.0f / lineDirection.x) : D3D12_FLOAT32_MAX,
        stepDirection.y != 0 ? std::abs(1.0f / lineDirection.y) : D3D12_FLOAT32_MAX,
        stepDirection.z != 0 ? std::abs(1.0f / lineDirection.z) : D3D12_FLOAT32_MAX
    );

    DirectX::XMFLOAT3 tMax = DirectX::XMFLOAT3(
        0,
        0,
        0
    );
    DirectX::XMINT3 traverseVoxel = startVoxel;

    while (memcmp(&traverseVoxel, &endVoxel, sizeof(DirectX::XMINT3)) != 0)//THIS IS CORRECT
    {
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            traverseVoxel.x += stepDirection.x;
            tMax.x += stepLength.x;
        }
        else if (tMax.y < tMax.z) {
            traverseVoxel.y += stepDirection.y;
            tMax.y += stepLength.y;
        }
        else {
            traverseVoxel.z += stepDirection.z;
            tMax.z += stepLength.z;
        }

        //GET WITH UV SHIT
        float procent = HF::distance(traverseVoxel, endVoxel) / HF::distance(startVoxel, endVoxel);
        DirectX::XMFLOAT2 uv(
            startUV.x + ((endUV.x - startUV.x) * procent),
            startUV.y + ((endUV.y - startUV.y) * procent)
        );
        const DirectX::XMUINT4 rgb = ColorFromUVAndTexture(uv, Texture);

        int index = getIndex(traverseVoxel.x, traverseVoxel.y, traverseVoxel.z >= sizes.z ? sizes.z - 1 : traverseVoxel.z, sizes.x, sizes.y);

        if (rgb.w == 0)
        {
            continue;
        }

        voxelGrid[index].rgb[0] = rgb.x;
        voxelGrid[index].rgb[1] = rgb.y;
        voxelGrid[index].rgb[2] = rgb.z;

        if (rgb.x + rgb.y + rgb.z == 0)//If it's black just add one bcs 0 should be nothing
        {
            voxelGrid[index].rgb[0] = 1;
        }
    }
}

void ModelToVoxel::errorAndWarningHandler(const uint32_t error)
{
    switch(error){
    case 1:
        information = "Error : Cannot find model.";
        break;
    }
}

VoxelModel::~VoxelModel()
{
}

TextureForVoxels::~TextureForVoxels()
{
    delete data;
}
