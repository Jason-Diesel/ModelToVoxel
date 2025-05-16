#include "Engine.h"

Engine::Engine() 
	:
	shaderHandler(&gfx, &renderer, &lightHandler),
	renderer(&gfx),
	lightHandler(&gfx),
	imguiHandler(&gfx, &window, &sceneManager),
	fileReader(&gfx, &rm)
{
//enable console
#ifdef _DEBUG
	RedirectIOToConsole();
#endif
	init();
}

Engine::Engine(Scene* startScene)
	:
	shaderHandler(&gfx, &renderer, &lightHandler),
	renderer(&gfx),
	lightHandler(&gfx),
	imguiHandler(&gfx, &window, &sceneManager),
	fileReader(&gfx, &rm)
{
	//enable console
#ifdef _DEBUG
	RedirectIOToConsole();
#endif
	sceneManager.setStartScene(startScene);
	init();
}

Engine::~Engine()
{
	sceneManager.Destroy();
	rm.Destroy();
}

void Engine::setStartScene(Scene* startScene)
{
	sceneManager.setStartScene(startScene);
}

void Engine::start()
{
	sceneManager.getCurrentScene()->Start();
	update();
}

void Engine::init()
{
	window.Initialize(GetModuleHandle(NULL), "Penis", "a", window.WindowWidth(), window.WindowHeight());
	
	window.getMouse().activateMouse(true);
	
	//TODO: Do I need mouse and keyboard here?!
	this->mouse = &window.getMouse();
	this->keyboard = &window.getKeyboard();
	
	gfx.init(window);
	shadowMapsManager.init(&gfx, &rm);
	shaderHandler.init();
	renderer.init();
	lightHandler.init();
	imguiHandler.init();
	
	Material* mat = new Material();
	DefaultMaterialData defmatData;
	mat->init(&gfx, 1, sizeof(defmatData));
	
	mat->addTexture(0, createTexture("../textures/7fJYp.png", &rm, &gfx), &gfx);
	rm.addResource(mat, "_Def_Material");
	
	{
		SceneHelperData sceneHelperData{
			.shaderHandler	= &this->shaderHandler,
			.sceneManager	= &this->sceneManager,
			.mouse			= &window.getMouse(),
			.keyboard		= &window.getKeyboard(),
			.gfx			= &this->gfx,
			.imguiHandler	= &this->imguiHandler,
			.renderer		= &this->renderer,
			.lights			= &this->lightHandler,
			.fileReader		= &this->fileReader,
			.rm				= &this->rm
		};
		sceneManager.setSceneHelperData(sceneHelperData);
	}
}

void Engine::update()
{
	while (msg.message != WM_QUIT && window.ProcessMessages())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//UPDATE SCENE
		sceneManager.update((float)deltaTime.dt());

		gfx.beginFrame();
	
	
		if (keyboard->onceisKeyReleased(VK_TAB))
		{
			mouse->activateMouse(!mouse->getMouseActive());
		}
	
		shadowMapsManager.update(lightHandler);
	
		//UPDATE LIGHT INFORMATION
		lightHandler.update();
	
		render();
	
		mouse->update();
		deltaTime.restartClock();
	}
}

void Engine::render()
{
	//SET SHADOWMAP SHADER
	renderer.ShadowReady();
	shaderHandler.setShadowShaders(0);
	shadowMapsManager.setAsDepthPass();
	shadowMapsManager.clearDeapthStencils();
	
	
	//RENDER SHADOWS HERE//
	uint32_t shadowMapIndex = 0;
	for (int i = 0; i < lightHandler.getNrOfActiveLights(); i++)
	{
		if (lightHandler.getLight(i)->getLightType() == LightType::PointLight_E)
		{
			continue;
		}
		
		shadowMapsManager.SetAsDepthStencil(shadowMapIndex);
		lightHandler.setLightAsPOV(i);
		sceneManager.render();
		shadowMapIndex++;
	}
	//////////////////////
	
	shadowMapsManager.setAsShadowPass();
	renderer.setReady();

	shaderHandler.setShader(1);

	gfx.setNormalRenderTarget();
	gfx.setNormalViewPort();
	
	if (keyboard->isKeyPressed('G'))
	{
		lightHandler.setLightAsPOV(0);
	}
	
	//RENDER THINGS AS NORMAL
	sceneManager.render();
	
	//RENDER IMGUI
	imguiHandler.startRender();
	if (!mouse->getMouseActive())
	{
		imguiHandler.renderUI();
	}

	imguiHandler.renderUIFromScene();
	imguiHandler.endRender();

	gfx.endFrame();
}
