#pragma once
#include "CommonHeaders.h"
#include "ConstantBuffers.h"

enum LightType {
	PointLight_E = 0,
	DirectionalLight_E,
	SpotLight_E,
	CascadeLight_E,
};

class Light {
public:
	Light(const DirectX::XMFLOAT3& position);
	virtual DirectX::XMMATRIX getLightViewProjection();
	virtual const ViewProjConstantBufferData getLightViewProjectionBufferData();
	const LightType getLightType() const;
	const DirectX::XMFLOAT3& getPosition();
	const DirectX::XMFLOAT4& getColor();
	virtual const DirectX::XMFLOAT3& getRotation();
	void setPosition(const DirectX::XMFLOAT3& position);
	void setColor(const DirectX::XMFLOAT4& color);
	void setColor(const DirectX::XMFLOAT3& color);//DO NOT BRING ALPHA
	virtual void setRotation(const DirectX::XMFLOAT3& rotation);
	const DirectX::XMUINT2& getShadowMapSize();
	void setShadowMapSizeInit(const DirectX::XMUINT2& sizes);//THIS IS NOT POSSIBLE TO AFTER THE INIT OF THE LIGHT IN SHADOW MAP
	int& getShadowMapPos();
	void setShadowSoftNess(const uint32_t shadowSoftNess);
	const uint32_t getShadowMapSoftness() const;
protected:
	LightType lightType;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 position;

	DirectX::XMUINT2 shadowMapSize;
	int shadowMapPos;
	uint32_t shadowSoftness = 0;
};