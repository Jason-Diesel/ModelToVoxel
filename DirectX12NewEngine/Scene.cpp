#include "Scene.h"

//#include "ShaderHandler.h"
//#include "SceneManager.h"
//#include "WindowClass.h"

Scene::Scene()
{
}

Scene::~Scene()
{
	lights->reset();
}

void Scene::addEngineData(SceneHelperData& sceneHelperData)
{
	this->shaderHandler = sceneHelperData.shaderHandler;
	this->sceneManager	= sceneHelperData.sceneManager;
	this->mouse			= sceneHelperData.mouse;
	this->keyboard		= sceneHelperData.keyboard;
	this->gfx			= sceneHelperData.gfx;
	this->renderer		= sceneHelperData.renderer;
	this->lights		= sceneHelperData.lights;
	this->imguiHandler	= sceneHelperData.imguiHandler;
	this->fileReader	= sceneHelperData.fileReader;
	this->rm			= sceneHelperData.rm;

	this->renderer->setCurrentCamera(&camera);
	this->objectManager.init(sceneHelperData.gfx);
}

void Scene::Start()
{
}

void Scene::Update(const float& dt)
{
}

void Scene::Render()
{
}

void Scene::Destroy()
{
}

void Scene::RenderUI()
{
}
