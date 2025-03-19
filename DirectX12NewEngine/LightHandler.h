#pragma once
#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"
#include "ConstantBuffers.h"

class LightHandler {
public:
	LightHandler(Graphics* gfx);
	~LightHandler();
	void reset();
	void init();
	Light* addLight(
		const LightType lightType, 
		const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(0,0,0), 
		const DirectX::XMFLOAT3& rotation = DirectX::XMFLOAT3(0, 0, 0)
	);
	void addLight(Light* light);
	Light* getLight(const uint32_t index);
	void update();
	void setLightAsPOV(uint32_t lightIndex);
	const void setAsGraphicRoot() const;
	const uint16_t getNrOfActiveLights() const;
	const uint16_t getNrOfActiveShadowMaps() const;
private:
	ConstantBuffer lightBuffer;
	LightConstantBufferData lightData;

	ConstantBuffer lightPOVConstantBuffers[MAXNROFLIGHTS];
	CD3DX12_VIEWPORT dynamicViewPort;

	uint16_t nrOfLights;
	uint16_t nrOfShadowMaps;
	Light* lights[MAXNROFLIGHTS];

	Graphics* gfx;
};