#pragma once
#include "Scene.h"
#include "ReadModelFile.h"
#include "SpotLight.h"

struct Vertecies {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
};

struct Voxel {
	int rgb[3] = { 0, 0, 0 };
};

class TestScene : public Scene
{
public:
	TestScene();
	~TestScene();
	void Start();
	void Update(const float& dt);
	void Render();
	void RenderUI();
	
private:
	uint32_t debugShader;
	Voxel* Rasterize();
	void addABox(const DirectX::XMFLOAT3 offset, std::vector<Vertex>& vertecies, std::vector<uint32_t>& indecies);
	void LoadModel(
		std::vector<Vertecies>& vertecies,
		std::vector<uint32_t>& indecies
	);
	void getBox();
	Model* StartModel;

	std::vector<Vertex> boxVertecies;
	std::vector<uint32_t> boxIndecies;

	Light* testLight;
	std::string fileName = "...";
	DirectX::XMINT3 sizes = DirectX::XMINT3(10,10,10);
	DirectX::XMFLOAT3 boundingBox[2]; //0 = lowest, 1 = highest
};