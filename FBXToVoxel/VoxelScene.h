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
	TextureHeap VoxelPosition;
	static const uint32_t chunkSize = 200;

	Model* theVoxelModel;
	Model* GetBoxModel(const int size, const int NrOfBlocks);

	uint32_t voxelShader;
	Shader* shaderPtrForVoxel;
	Shader* shaderPtrForShadowVoxel;

	std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, Chunk*>>> chunks;
	//Chunk* testChunk;

	uint32_t LightObject, LightObject2;

	DirectX::XMFLOAT3 spinAround;
};
