#pragma once
#include "Object.h"
#include "Model.h"
#include "AnimationComponent.h"
#include "Graphics.h"
#include "Camera.h"
#include "Lights.h"
#include "ConstantBuffers.h"

//GONNA MAKE THIS SMARTER LATER BUT FOR NOW JUST FUCKING DRAW THE THING

class Renderer
{
public:
	Renderer(Graphics* gfx);
	~Renderer();
	void init();
	void ShadowReady();
	void setReady();//Do this everytime 
	void updateShader();//Do this once every frame //hopefully

	void render(Object* object, Model* customModel);
	void render(Object* object);
	void render(Model* model);
	void render(Mesh& mesh);

	void renderNrOfMeshes(Object* object, uint32_t nrOfMeshes);

	void setCurrentCamera(Camera* camera);
	const bool isMakingShadows();
private:
	bool shadowMode;
	Camera* currentCamera;
	Graphics* gfx;

	//Move these to camera?
	ConstantBuffer viewProjectionBuffer;
	ConstantBuffer transformBuffer;
};