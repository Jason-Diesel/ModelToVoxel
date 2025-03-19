#pragma once
#include "Scene.h"
#include "ReadModelFile.h"

#include "SpotLight.h"

class ComputeShaderTestScene : public Scene
{
public:
	ComputeShaderTestScene();
	~ComputeShaderTestScene();
	void Start();
	void Update(const float& dt);
	void Render();
private:
	Light* testLight;

	uint32_t computeShader;
	TextureHeap UAVTextureHeap;
	TextureViewClass* tvcUAV;
};