#include "TestScene.h"
#include "Vertex.h"
#include "VertexBufferCreator.h"

#include "TextureChanges.h"

#include <windows.h>
#include <commdlg.h>

#include <unordered_map>
#include <unordered_set>

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

TestScene::TestScene()
{
}

TestScene::~TestScene()
{
}

void TestScene::Start()
{
    objectManager.createAnObject();
    std::vector<MaterialDescription> empty;
    empty.push_back(MaterialDescription());
    debugShader = shaderHandler->createShader(
        0,
        empty,
        gfx->getInputLayout(0),
        "VertexShader.cso",
        "DebugPixel.cso",
        true
    );
    getBox();
}

void TestScene::Update(const float& dt)
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

void TestScene::Render()
{
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

void TestScene::RenderUI()
{
	if (ImGui::Begin("Start")) 
	{
        ImGui::InputInt3("Voxel Grid Size", (int*)&sizes);
		if (ImGui::Button("Open File Dialog"))
		{
			OpenFileDialog(this->fileName);
		}
        if (ImGui::Button("Start"))
        {
            StartModel = new Model();
            StartModel->nrOfSubMeshes = 1;
            StartModel->subMeshes = new Mesh[1];

            std::vector<Vertex> vertecies;
            std::vector<uint32_t> indecies;
            Voxel* voxels = this->Rasterize();
            int i = 0;
            for (int x = 0; x < sizes.x; x++)
            {
                for (int y = 0; y < sizes.y; y++)
                {
                    for (int z = 0; z < sizes.z; z++)
                    {
                        if (voxels[i++].rgb[0] != 0)
                        {
                            addABox(DirectX::XMFLOAT3(x, y, z), vertecies, indecies);
                        }
                    }
                }
            }
            StartModel->subMeshes[0].nrOfIndecies = indecies.size();
            StartModel->subMeshes[0].vertexBuffer = createVertexBuffer(
                vertecies,
                gfx->getDevice(),
                gfx->getCommandList(1),
                gfx->getCommandAllocator(1),
                gfx->getCommandQueue(),
                gfx->getFenceEvent(),
                gfx->getFence(),
                gfx->getFenceValue()
            );
            StartModel->subMeshes[0].indeciesBuffer = createIndeciesBuffer(
                indecies,
                gfx->getDevice(),
                gfx->getCommandList(1),//unsure If I should I have a number here?
                gfx->getCommandAllocator(1),//unsure If I should I have a number here?
                gfx->getCommandQueue(),
                gfx->getFenceEvent(),
                gfx->getFence(),
                gfx->getFenceValue()
            );
            StartModel->subMeshes[0].vertexBufferView.BufferLocation = StartModel->subMeshes[0].vertexBuffer->GetGPUVirtualAddress();
            StartModel->subMeshes[0].vertexBufferView.SizeInBytes = (uint32_t)vertecies.size() * sizeof(Vertex);
            StartModel->subMeshes[0].vertexBufferView.StrideInBytes = sizeof(Vertex);

            StartModel->subMeshes[0].indeciesBufferView.BufferLocation = StartModel->subMeshes[0].indeciesBuffer->GetGPUVirtualAddress();
            StartModel->subMeshes[0].indeciesBufferView.SizeInBytes = (UINT)(indecies.size() * sizeof(uint32_t));
            StartModel->subMeshes[0].indeciesBufferView.Format = DXGI_FORMAT_R32_UINT;

            StartModel->subMeshes[0].material = rm->getResource<Material>("_Def_Material");

            objectManager.getObject(0)->addComponent(StartModel);
        }
	}	
	ImGui::End();
}

void TestScene::LoadModel(
    std::vector<Vertecies>& vertecies,
    std::vector<uint32_t>& indecies
)
{
    std::cout << fileName << std::endl;
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
    
    boundingBox[0] = DirectX::XMFLOAT3(D3D12_FLOAT32_MAX, D3D12_FLOAT32_MAX, D3D12_FLOAT32_MAX);
    boundingBox[1] = DirectX::XMFLOAT3(-D3D12_FLOAT32_MAX, -D3D12_FLOAT32_MAX, -D3D12_FLOAT32_MAX);//I FUCKING HOPE
    
    for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
    {
        aiMesh* mesh = pScene->mMeshes[i];
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

            vertecies.push_back(vertex);
        }
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            indecies.push_back(mesh->mFaces[i].mIndices[0]);
            indecies.push_back(mesh->mFaces[i].mIndices[1]);
            indecies.push_back(mesh->mFaces[i].mIndices[2]);
        }
    }
    //GONNA NEED TO LOAD MATERIAL HERE ALSO
}

Voxel* TestScene::Rasterize() {
    std::vector<Vertecies> vertecies;
    std::vector<uint32_t> indecies;

    LoadModel(vertecies, indecies);

    //ACTUALLY DO RASTERIZE
    Voxel* voxelGrid = new Voxel[sizes.x * sizes.y * sizes.z];

    float voxelSize = (boundingBox[1].x - boundingBox[0].x) / sizes.x;
    voxelSize = max(voxelSize, (boundingBox[1].y - boundingBox[0].y)/ sizes.y);
    voxelSize = max(voxelSize, (boundingBox[1].z - boundingBox[0].z)/sizes.z);
    voxelSize += 0.0000001f;//BIAS IS NEEDED
    const DirectX::XMFLOAT3 minSizes = boundingBox[0];

    for (uint32_t i = 0; i < indecies.size(); i += 3)
    {
        //FOR EACH TRIANGLE
        //Localize Vertex Points
        DirectX::XMINT3 voxelPosition[3];
        uint32_t low = UINT32_MAX, high = 0;//HEIGHT BOUNDING LINE
        for (uint8_t j = 0; j < 3; j++)
        {
            const DirectX::XMFLOAT3 vetexPos = vertecies[indecies[i + j]].position;
            //DO THIS WITH VECTORS INSTEAD OF XMINT3
            DirectX::XMFLOAT3 positionInGrid = div(sub(vetexPos, minSizes), voxelSize);
            voxelPosition[j] = F3ToI3(positionInGrid);

            low = min(voxelPosition[j].y, low);
            high = max(voxelPosition[j].y, high);

            //ADD VALUE TO VOXEL 
            int index = getIndex(voxelPosition[j].x, voxelPosition[j].y, voxelPosition[j].z, sizes.x, sizes.y);
            voxelGrid[index].rgb[0] = 1;//FOR NOW JUST HERE
        }
        
        //unordered_map<Y, std::vector<X,Z>>
        std::unordered_map<int, std::unordered_set<std::pair<int, int>>> BlocksForLineDrawing;
        //Create lines from each vertex
        for (int j = 0; j < 3; j++)
        {
            if (i == 12 && j == 2)
            {
                std::cout << "stop" << std::endl;//DEBUG THIS SHIT
            }
            const DirectX::XMINT3 endVoxel = voxelPosition[(j + 1) % 3];
            DirectX::XMINT3 distance = DirectX::XMINT3(
                endVoxel.x - voxelPosition[j].x,
                endVoxel.y - voxelPosition[j].y,
                endVoxel.z - voxelPosition[j].z
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
            DirectX::XMINT3 traverseVoxel = voxelPosition[j];

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
                int index = getIndex(traverseVoxel.x, traverseVoxel.y, traverseVoxel.z, sizes.x, sizes.y);
                voxelGrid[index].rgb[0] = 1;//FOR NOW JUST HERE

                //BlocksForLineDrawing
            }
        }
    }

    //GONNA NEED TO DO FILL LATER

    int i = 0;
    for (int x = 0; x < sizes.x; x++) {
        for (int y = 0; y < sizes.y; y++) {
            for (int z = 0; z < sizes.z; z++) {
                std::cout << voxelGrid[i++].rgb[0] << ", ";
            }
            std::cout << "\n";
        }
        std::cout << "------------------------------------" << std::endl;
    }
    //delete[] voxelGrid;
    return voxelGrid;
}



void TestScene::addABox(const DirectX::XMFLOAT3 offset, std::vector<Vertex>& vertecies, std::vector<uint32_t>& indecies)
{
    for (int i = 0; i < boxVertecies.size(); i++)
    {
        Vertex v = boxVertecies[i];
        v.pos.x += offset.x;
        v.pos.y += offset.y;
        v.pos.z += offset.z;
        vertecies.push_back(v);
    }
    const uint32_t startSize = indecies.size();
    for (uint32_t i = 0; i < boxIndecies.size(); i++) {
        indecies.push_back(boxIndecies[i] + startSize);
    }
}

void TestScene::getBox() {
    const std::string path = "C:/Users/etta1/source/repos/3DObjectsToVoxels/Models/TestBox.fbx";
    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(path.c_str(),
        aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs
    );

    if (!pScene) {
        std::wcout << L"couldn't find " << fileName.c_str() << " in directory: " << HF::getCurrentDirectory() << std::endl;
        printf("Error");
        return;
    }
    for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
    {
        aiMesh* pMesh = pScene->mMeshes[i];
        for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
        {
            Vertex vert;
            vert.pos.x = pMesh->mVertices[i].x/2.0f;
            vert.pos.y = pMesh->mVertices[i].y/2.0f;
            vert.pos.z = pMesh->mVertices[i].z/2.0f;

            vert.normal.x = pMesh->mNormals[i].x;
            vert.normal.y = pMesh->mNormals[i].y;
            vert.normal.z = pMesh->mNormals[i].z;

            if (pMesh->mTextureCoords[0] != nullptr)
            {
                vert.uv.x = pMesh->mTextureCoords[0][i].x;
                vert.uv.y = pMesh->mTextureCoords[0][i].y;
            }
            else {
                vert.uv.x = 0;
                vert.uv.y = 0;
            }
            this->boxVertecies.push_back(vert);
        }
        for (uint32_t i = 0; i < pMesh->mNumFaces; i++) {
            boxIndecies.push_back(pMesh->mFaces[i].mIndices[0]);
            boxIndecies.push_back(pMesh->mFaces[i].mIndices[1]);
            boxIndecies.push_back(pMesh->mFaces[i].mIndices[2]);
        }
    }
}

