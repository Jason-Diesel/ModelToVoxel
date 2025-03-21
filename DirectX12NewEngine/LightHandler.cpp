#include "LightHandler.h"

LightHandler::LightHandler(Graphics* gfx):
	gfx(gfx),
	nrOfLights(0)
{
	dynamicViewPort.MaxDepth = 1.0f;
	dynamicViewPort.MinDepth = 0.0f;
}

LightHandler::~LightHandler()
{
	lightBuffer.constantBuffer->Release();
	for (int i = 0; i < MAXNROFLIGHTS; i++)
	{
		lightPOVConstantBuffers[i].constantBuffer->Release();
	}
	reset();
}

void LightHandler::reset()
{
	for (int i = 0; i < nrOfLights; i++)
	{
		delete lights[i];
	}
	nrOfLights = 0;
	nrOfShadowMaps = 0;
}

void LightHandler::init()
{
	lightBuffer = CreateConstantBuffer<LightConstantBufferData>(gfx);
	for (int i = 0; i < MAXNROFLIGHTS; i++)
	{
		lightPOVConstantBuffers[i] = CreateConstantBuffer<ViewProjConstantBufferData>(gfx);
	}
}

Light* LightHandler::addLight(
	const LightType lightType, 
	const DirectX::XMFLOAT3& position, 
	const DirectX::XMFLOAT3& rotation
)
{
	
	switch (lightType) {
	case PointLight_E:
		lightData.LightPosoLightType[nrOfLights].w = PointLight_E;
		lights[nrOfLights++] = new PointLight(position);
		break;
	case DirectionalLight_E:
		lightData.LightPosoLightType[nrOfLights].w = DirectionalLight_E;
		lights[nrOfLights++] = new DirectionalLight(position, rotation);
		nrOfShadowMaps++;
		break;
	case SpotLight_E:
		lightData.LightPosoLightType[nrOfLights].w = SpotLight_E;
		lights[nrOfLights++] = new SpotLight(position, rotation);
		nrOfShadowMaps++;
		break;
	};

	return lights[nrOfLights - 1];
}

void LightHandler::addLight(Light* light)
{
	lightData.LightPosoLightType[nrOfLights].w = light->getLightType();
	lights[nrOfLights++] = light;
	if (lightData.LightPosoLightType[nrOfLights].w != PointLight_E)
	{
		nrOfShadowMaps++;
	}
}

Light* LightHandler::getLight(const uint32_t index)
{
	return this->lights[index];
}

void LightHandler::update()
{
	lightData.nrOfLights = nrOfLights;
	for (int i = 0; i < nrOfLights; i++)
	{
		memcpy(&lightData.LightPosoLightType[i], &lights[i]->getPosition(), sizeof(DirectX::XMFLOAT3));
		lightData.ViewProjectionMatrix[i] = lights[i]->getLightViewProjection();
		lightData.LightColor[i] = lights[i]->getColor();
		lightData.ShadowMapInfo[i].x = lights[i]->getShadowMapPos();		//shadowMap Index in array
		lightData.ShadowMapInfo[i].y = lights[i]->getShadowMapSoftness();	//shadowMap softness
	}
	updateConstantBuffer(lightData, lightBuffer);
	
}

void LightHandler::setLightAsPOV(uint32_t lightIndex)
{
	//SET CONSTANTBUFFER ON POS 0 a.k.a camera matrices
	updateConstantBuffer(lights[lightIndex]->getLightViewProjectionBufferData(), lightPOVConstantBuffers[lightIndex]);
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(0, lightPOVConstantBuffers[lightIndex].constantBuffer->GetGPUVirtualAddress());

	//Set View Port
	dynamicViewPort.Width = lights[lightIndex]->getShadowMapSize().x;
	dynamicViewPort.Height = lights[lightIndex]->getShadowMapSize().y;
	gfx->getCommandList()->RSSetViewports(1, &this->dynamicViewPort);
	this->currentLight = lightIndex;
}

void LightHandler::setCurrentLightAsPOV()
{
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(0, lightPOVConstantBuffers[currentLight].constantBuffer->GetGPUVirtualAddress());
}

const void LightHandler::setAsGraphicRoot() const
{
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(3, lightBuffer.constantBuffer->GetGPUVirtualAddress());
}

const uint16_t LightHandler::getNrOfActiveLights() const
{
	return nrOfLights;
}

const uint16_t LightHandler::getNrOfActiveShadowMaps() const
{
	return this->nrOfShadowMaps;
}
