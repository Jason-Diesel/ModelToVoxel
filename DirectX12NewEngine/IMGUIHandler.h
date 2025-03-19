#pragma once
#include "Lights.h"
#include "Object.h"
#include "Graphics.h"

#include "Library/Imgui/imgui.h"
#include "Library/Imgui/imgui_impl_win32.h"
#include "Library/Imgui/imgui_impl_dx12.h"

class SceneManager;

class IMGUIHandler
{
public:
	IMGUIHandler(Graphics* gfx, Window* window, SceneManager* sceneManager);
	~IMGUIHandler();
	void init();

	void startRender();
	void endRender();

	void renderUI();
	void renderUIFromScene();

	void addObject(Object* object);
	void addLight(Light* light);
private:
	std::vector<Object*> objects;
	std::vector<Light*> lights;

	Object* activeObject = nullptr;
	Light* activeLight = nullptr;
	SceneManager* sceneManager;
	Graphics* gfx;
	Window* window;
};