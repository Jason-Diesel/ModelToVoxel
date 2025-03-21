#pragma once
#include "Engine.h"
#include "ReadWriteVoxels.h"
#include "Chunk.h"



class VoxelScene : public Scene {
public:
	VoxelScene();
	~VoxelScene();
	void Start();
	void Update(const float& dt);
	void Render();
	void RenderUI();
private:
	static const uint32_t chunkSize = 256;
	Model* voxelModels[NROFLOD];
	Model* GetVoxelModel(const int size, const int NrOfBlocks);

	Shader* shaderPtrForVoxel;
	Shader* shaderPtrForShadowVoxel;
	uint32_t voxelMinimizerComputeShader;

	std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, Chunk*>>> chunks;
	//Chunk* testChunk;

	uint32_t LightObject, LightObject2;

	DirectX::XMFLOAT3 spinAround;
};
