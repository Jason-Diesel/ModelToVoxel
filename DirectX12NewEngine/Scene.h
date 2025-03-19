#pragma once
#include <stdint.h>
#include "Object.h"
#include "Model.h"
#include "Graphics.h"
#include "Renderer.h"
#include "LightHandler.h"
#include "IMGUIHandler.h"
#include "ObjectManager.h"
#include "ReadModelFile.h"
#include "ResourceManager.h"
#include "ShaderHandler.h"

//class ShaderHandler;
class SceneManager;
class Mouse;
class Keyboard;
class IMGUIHandler;


enum SceneReturn
{
	NOTHING,
	QUIT,
};

struct SceneHelperData {
	ShaderHandler* shaderHandler;
	SceneManager* sceneManager;
	Mouse* mouse;
	Keyboard* keyboard;
	Graphics* gfx;
	IMGUIHandler* imguiHandler;
	Renderer* renderer;
	LightHandler* lights;
	FileReader* fileReader;
	ResourceManager* rm;
};

class Scene {
public:
	Scene();
	virtual ~Scene();
	void addEngineData(SceneHelperData& sceneHelperData);
	virtual void Start();
	virtual void Update(const float& dt);
	virtual void Render();
	virtual void Destroy();
	virtual void RenderUI();
protected:
	ShaderHandler* shaderHandler;
	SceneManager* sceneManager;
	Renderer* renderer;
	Mouse* mouse;
	Keyboard* keyboard;
	Graphics* gfx;
	LightHandler* lights;
	IMGUIHandler* imguiHandler;
	FileReader* fileReader;
	//Unsure how much we need this in the scene?
	//But just to be sure right now
	ResourceManager* rm;

	ObjectManager objectManager;
protected:
	Camera camera;
private:
	
};