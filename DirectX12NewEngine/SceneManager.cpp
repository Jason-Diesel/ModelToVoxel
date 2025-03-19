#include "SceneManager.h"

SceneManager::SceneManager():
	currentScene(nullptr),
	nextScene(nullptr)
{
}

SceneManager::SceneManager(Scene* startScene):
	currentScene(startScene),
	nextScene(nullptr)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::Destroy()
{
	if (currentScene != nullptr)
	{
		delete currentScene;
	}
	if (nextScene != nullptr)
	{
		delete nextScene;
	}
}

void SceneManager::setStartScene(Scene* startScene)
{
	currentScene = startScene;
}

void SceneManager::setScene(Scene* scene)
{
	nextScene = scene;
}

void SceneManager::update(const float& dt)
{
	if (nextScene != nullptr)
	{
		currentScene->Destroy();
		delete currentScene;

		//RESTART CommandAllocator and commandlist

		currentScene = nextScene;
		nextScene = nullptr;

		currentScene->addEngineData(sceneHelperData);
		currentScene->Start();
	}

	currentScene->Update(dt);
	
}

void SceneManager::render()
{
	currentScene->Render();
}

void SceneManager::setSceneHelperData(SceneHelperData& sceneHelperData)
{
	this->sceneHelperData = sceneHelperData;
	if (currentScene != nullptr)
	{
		currentScene->addEngineData(sceneHelperData);
	}
}

Scene* SceneManager::getCurrentScene()
{
	return currentScene;
}
