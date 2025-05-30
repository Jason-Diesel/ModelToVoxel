#pragma once
#include "Engine.h"
#include "ReadWriteVoxels.h"
#include "ReadBackBuffer.h"
#include "GraphicsBufferWithData.h"
#include "Chunk.h"

#include "SharedVoxelFunctions.h"

struct TextureForVoxels {
	~TextureForVoxels();
	uint32_t width = 0;
	uint32_t height = 0;
	DirectX::XMFLOAT2 uvIncrement;
	uint32_t* data;//RGB
};

struct VoxelMesh {
	std::vector<Vertecies> vertecies;
	std::vector<uint32_t> indecies;
	uint32_t materialIndex;
};

struct VoxelModel {
	
	~VoxelModel();
	std::vector<VoxelMesh> meshes;
	std::vector<TextureViewClass*> texturesGPU;
	std::vector<TextureForVoxels*> texturesCPU;
	std::vector<DirectX::XMINT4> IndeciesStartAndEnd;//Start indecies, end indecies, MaterialIndex On CPU, Material IndexOnGPU
};

class ModelToVoxel : public Scene
{
public:
	ModelToVoxel();
	~ModelToVoxel();
	void Start();
	void Update(const float& dt);
	void Render();
	void RenderUI();

private:
	uint32_t debugShader;

	Model* StartModel;

	std::string inputFileName = "";
	std::string outPutFileName = "../Models/VoxelTest.vox";
	std::string information = "";
	DirectX::XMUINT3 sizes = DirectX::XMUINT3(200, 200, 200);
	DirectX::XMFLOAT3 boundingBox[2]; //0 = lowest, 1 = highest


	void lineToLine(
		const DirectX::XMINT3& startVoxel,
		const DirectX::XMINT3& endVoxel,
		const DirectX::XMFLOAT2& startUV,
		const DirectX::XMFLOAT2& endUV,
		const TextureForVoxels* Texture,
		Voxel* voxelGrid
	);
	void errorAndWarningHandler(const uint32_t error);

	//CPU SHIT
	Voxel* CreateVoxelModelCPU();
	void LoadModelForCPU(VoxelModel& theReturn, ResourceManager* rm);
	void TriangleLoading(
		const VoxelModel& model, 
		const uint32_t start,
		const uint32_t offset, //nrOfThreads
		const uint32_t m,//Mesh 
		const float voxelSize,
		const DirectX::XMFLOAT3& minSizes,
		Voxel* voxelGrid//We don't actually care much about if something comes a little bit first here
	);
	
	TextureForVoxels* LoadTexture(const std::string& filePath);
	DirectX::XMUINT4 ColorFromUVAndTexture(DirectX::XMFLOAT2 UV, const TextureForVoxels* texture);

	//GPU SHIT
	uint32_t computeVoxelsShader;
	TextureHeap rbBufferHeap;
	ReadBackBuffer* rbBuffer;//only the VoxelData here
	
	ConstantBuffer creatingVoxelModelData;

	Voxel* CreateVoxelModelGPU();
	void LoadModelForGPU(VoxelModel& theReturn, ResourceManager* rm);
	bool doneShit = false;

	struct {
		DirectX::XMUINT4 sizes;
		DirectX::XMFLOAT4 minSizes;
		DirectX::XMFLOAT4 voxelSize;
	}creatingVoxelModelDataData;


	//Voxel Model Showing
	static const uint32_t chunkSize = 256;
	std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, Chunk*>>> chunks;

	Model* voxelModels;

	Shader* shaderPtrForVoxel;
	Shader* shaderPtrForShadowVoxel;
	uint32_t voxelMinimizerComputeShader;

	TextureHeap translationTextureHeapUAV;

	DirectX::XMFLOAT3 spinAround;
	float distanceFromMiddle = 10;
	float cameraHeight = 0;

	void createRenderingVoxelModel(const Voxel* voxels, const DirectX::XMUINT3& sizes);
};

void OpenFileDialog(std::string& fileName);