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
    std::vector<MaterialDescription> empty;
    empty.push_back(MaterialDescription({1}));
    debugShader = shaderHandler->createShader(
        0,
        empty,
        gfx->getInputLayout(0),
        "VertexShader.cso",
        "DebugPixel.cso",
        true
    );

    uint32_t data = 5;
    rbBuffer = new ReadBackBuffer(
        &data, 
        sizeof(uint32_t),
        gfx
    );
    computeVoxelsShader = shaderHandler->createShader(0, empty, "ComputeVoxels.cso");
    rbBufferHeap.init(1, gfx->getDevice());
    rbBufferHeap.createUAV(rbBuffer->getUAVResource(), gfx->getDevice());

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
    if (doneShit)
    {
        uint32_t* data = this->rbBuffer->getData<uint32_t>(gfx);
    }
    

    if (StartModel != nullptr)
    {
        shaderHandler->setShader(debugShader);
        renderer->render(objectManager.getObject(0));
    }
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
    if (ImGui::Begin("Voxel"))
    {

        ImGui::InputInt3("Voxel Grid Size", (int*)&sizes);
        if (ImGui::Button("Open File Dialog"))
        {
            OpenFileDialog(this->fileName);
        }
        if (ImGui::Button("Start Creating VoxelModel GPU"))
        {
            VoxelModel model;
            LoadModelForGPU(model, rm);
            struct {
                uint32_t nrOfIndecies;
                DirectX::XMFLOAT3 boundingBoxes[2];
                uint32_t materialIndex;
                float voxelSize;
            }IBMI;

            shaderHandler->setComputeShader(this->computeVoxelsShader);
            ID3D12DescriptorHeap* heaps[] = { rbBufferHeap.getHeap() };
            gfx->getCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);

            for (int m = 0; m < model.meshes.size(); m++)
            {
                
                if (model.texturesGPU[model.meshes[m].materialIndex] != nullptr)
                {
                    IBMI.materialIndex = model.meshes[m].materialIndex;
                    IBMI.boundingBoxes[0] = boundingBox[0];
                    IBMI.boundingBoxes[1] = boundingBox[1];
                    IBMI.nrOfIndecies = model.meshes[m].indecies.size();
                    IBMI.voxelSize = (boundingBox[1].x - boundingBox[0].x) / sizes.x;
                    IBMI.voxelSize = max(IBMI.voxelSize, (boundingBox[1].y - boundingBox[0].y) / sizes.y);
                    IBMI.voxelSize = max(IBMI.voxelSize, (boundingBox[1].z - boundingBox[0].z) / sizes.z);
                    IBMI.voxelSize += 0.0000001f;//BIAS IS NEEDED

                    computeVoxelData.push_back(GraphicsBufferWithData());
                    computeVoxelData.back().init(
                        &IBMI,
                        sizeof(IBMI),
                        gfx
                    );
                    computeVoxelData.push_back(GraphicsBufferWithData());
                    computeVoxelData.back().init(
                        model.meshes[m].vertecies.data(),
                        sizeof(Vertecies) * model.meshes[m].vertecies.size(),
                        gfx
                        );
                    computeVoxelData.push_back(GraphicsBufferWithData());
                    computeVoxelData.back().init(
                        model.meshes[m].vertecies.data(),
                        sizeof(uint32_t)* model.meshes[m].vertecies.size(),
                        gfx
                    );


                    gfx->getCommandList()->SetComputeRootDescriptorTable(0, rbBufferHeap.getHeap()->GetGPUDescriptorHandleForHeapStart());
                    
                    gfx->getCommandList()->Dispatch(1, 1, 1);
                    doneShit = true;
                }
            }
        }
        if (ImGui::Button("Start Creating VoxelModel CPU"))
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
            information = "Sucessfully created VoxelModel";
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

    theReturn.meshes.resize(pScene->mNumMeshes);

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
                theReturn.texturesGPU[m] = createTexture(pathString, rm, gfx);
                rm->addResource(theReturn.texturesGPU[m], "VoxelTextures__" + pathString);
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
    information = "Loading Model...";
    LoadModelForCPU(model, rm);
    if (model.meshes.size() == 0)
    {
        return nullptr;
    }
    information = "Creating Voxel Model...";

    //Create a smaller version?
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

    

    const int nrOfThreads = (std::thread::hardware_concurrency() / 2) + 1;
    std::thread* threads = new std::thread[nrOfThreads];

    for (uint32_t m = 0; m < model.meshes.size(); m++) 
    {
        for (int i = 0; i < nrOfThreads; i++)
        {
            threads[i] = std::thread(&ModelToVoxel::TriangleLoading, this,
                model,
                i,
                nrOfThreads,
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
    }

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
