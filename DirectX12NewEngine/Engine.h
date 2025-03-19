#pragma once
#include "SceneManager.h"
#include "ShaderHandler.h"
#include "DeltaTime.h"
#include "ReadModelFile.h"
#include "Renderer.h"
#include "Graphics.h"
#include "IMGUIHandler.h"
#include "ShadowMaps.h"
#include "ResourceManager.h"
#include "ReadModelFile.h"

class Engine {
public:
	Engine();
	Engine(Scene* startScene);
	~Engine();
	void setStartScene(Scene* startScene);
	void start();
	void end();
private:
	void init();
	void update();

	void render();
private:
	MSG msg = {};

	ShaderHandler shaderHandler;
	ShadowMaps shadowMapsManager;
	SceneManager sceneManager;
	Renderer renderer;
	LightHandler lightHandler;
	IMGUIHandler imguiHandler;
	Graphics gfx;
	DeltaTime deltaTime;
	//
	FileReader fileReader;
	ResourceManager rm;
	
	Window window;
	Mouse* mouse;
	Keyboard* keyboard;

private:
};

