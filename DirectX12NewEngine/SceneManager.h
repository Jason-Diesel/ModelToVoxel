#pragma once
#include "Scene.h"

class SceneManager
{
public:
	SceneManager();
	SceneManager(Scene* startScene);
	~SceneManager();
	void Destroy();
	void setStartScene(Scene* startScene);
	void setScene(Scene* scene);
	void update(const float& dt);
	void render();
	void setSceneHelperData(SceneHelperData& sceneHelperData);
	Scene* getCurrentScene();
private:
	Scene* currentScene;
	Scene* nextScene;

	SceneHelperData sceneHelperData;
};