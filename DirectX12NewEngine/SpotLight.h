#pragma once
#include "Lights.h"

class SpotLight : public Light{
public:
	SpotLight(
		const DirectX::XMFLOAT3& position, 
		const DirectX::XMFLOAT3& rotation = DirectX::XMFLOAT3(0,0,0),
		const float& fov = 90,
		DirectX::XMUINT2 shadowMapSize = DirectX::XMUINT2(2000,2000),
		const float& farPlane = 2000
	);
	virtual DirectX::XMMATRIX getLightViewProjection();
	const ViewProjConstantBufferData getLightViewProjectionBufferData();
	const DirectX::XMFLOAT3& getRotation();
	virtual void setRotation(const DirectX::XMFLOAT3& rotation);
private:
	DirectX::XMFLOAT3 rotation;
	DirectX::XMMATRIX projection;
};