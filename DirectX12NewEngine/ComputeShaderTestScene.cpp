#include "ComputeShaderTestScene.h"
#include "Vertex.h"
#include "VertexBufferCreator.h"
#include "TextureChanges.h"

ComputeShaderTestScene::ComputeShaderTestScene()
{
}

ComputeShaderTestScene::~ComputeShaderTestScene()
{
}

void ComputeShaderTestScene::Start()
{
	std::vector<MaterialDescription> aMaterial;
	aMaterial.push_back(MaterialDescription({ 1 }));
	computeShader = shaderHandler->createShader(0, aMaterial, "ComputeShaderTest.cso");
	UAVTextureHeap.init(1, gfx->getDevice());


	float* testData = new float[6 * 4] {
		0.5f, 0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, 0.5f, 0.5f,
			0.5f, 0.5f, 0.5f, 0.5f
		};

	uint32_t size = sizeof(float) * 4;
	tvcUAV = createUAV((void*)testData, size, DirectX::XMINT2(6, 1), rm, gfx);
	gfx->getTextureHeap().createSRV(tvcUAV, gfx);
	UAVTextureHeap.createUAV(tvcUAV, gfx);


	Model* Sponza = fileReader->readModel("../Models/Sponza.obj.OFT");//1.7 sec

	objectManager.createAnObject();
	objectManager.getObject(0)->addComponent(Sponza);

	testLight = lights->addLight(LightType::SpotLight_E);
	testLight->setColor(DirectX::XMFLOAT3(1.f, 0.8f, 0.8f));
	testLight->setShadowSoftNess(3);

	imguiHandler->addObject(objectManager.getObject(0));
	imguiHandler->addObject(objectManager.getObject(1));
	imguiHandler->addLight(lights->getLight(0));
}

void ComputeShaderTestScene::Update(const float& dt)
{
	float speed = 20;
	if (keyboard->isKeyPressed(VK_CONTROL))
	{
		speed = 80;
	}
	else {
		speed = 20;
	}
	if (keyboard->isKeyPressed('A'))
	{
		camera.addPositionBasedOnDirection(DirectX::XMFLOAT3(1, 0, 0), speed * dt);
	}
	if (keyboard->isKeyPressed('W'))
	{
		camera.addPositionBasedOnDirection(DirectX::XMFLOAT3(0, 0, -1), speed * dt);
	}
	if (keyboard->isKeyPressed('D'))
	{
		camera.addPositionBasedOnDirection(DirectX::XMFLOAT3(-1, 0, 0), speed * dt);
	}
	if (keyboard->isKeyPressed('S'))
	{
		camera.addPositionBasedOnDirection(DirectX::XMFLOAT3(0, 0, 1), speed * dt);
	}
	if (keyboard->isKeyPressed(VK_SPACE))
	{
		camera.move(DirectX::XMFLOAT3(0, speed * dt, 0));
	}
	if (keyboard->isKeyPressed(VK_SHIFT))
	{
		camera.move(DirectX::XMFLOAT3(0, -speed * dt, 0));
	}

	camera.rotate(DirectX::XMFLOAT3(
		-(float)mouse->getDeltaPos().x * mouse->getSense(),
		(float)mouse->getDeltaPos().y * mouse->getSense(),
		0)
	);

	static int lightIndex = 0;
	if (keyboard->onceisKeyReleased('L'))
	{
		lightIndex++;
		lightIndex %= lights->getNrOfActiveLights();
	}

	if (keyboard->isKeyPressed('F'))
	{
		lights->getLight(lightIndex)->setPosition(camera.getPostion());
		lights->getLight(lightIndex)->setRotation(camera.getRotation());
	}
	static float nTest = 0;
	if (keyboard->onceisKeyReleased('C'))
	{
		nTest += 1;
		nTest = (float)((int)nTest % 2);
		camera.setLastFloat(nTest);
	}
}

void ComputeShaderTestScene::Render()
{
	if (!renderer->isMakingShadows())
	{
		shaderHandler->setComputeShader(computeShader);

		ID3D12DescriptorHeap* heaps[] = { UAVTextureHeap.getHeap() };
		gfx->getCommandList()->SetDescriptorHeaps(_countof(heaps), heaps);
		gfx->getCommandList()->SetComputeRootDescriptorTable(0, UAVTextureHeap.getHeap()->GetGPUDescriptorHandleForHeapStart());

		gfx->getCommandList()->Dispatch(1, 1, 1);
	}

	UAVtoSRV(tvcUAV, gfx);

	shaderHandler->setShader(1);
	renderer->render(objectManager.getObject(0));

	SRVtoUAV(tvcUAV, gfx);
}
