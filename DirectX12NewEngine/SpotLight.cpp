#include "SpotLight.h"
#include "HelperFuncitons.h"

SpotLight::SpotLight(
	const DirectX::XMFLOAT3& position, 
	const DirectX::XMFLOAT3& rotation,
	const float& fov,
	DirectX::XMUINT2 shadowMapSize,
	const float& farPlane
):
	Light(position), rotation(rotation)
{
	lightType = LightType::SpotLight_E;
	
	this->shadowMapSize = shadowMapSize;
	const float aspectRatio = shadowMapSize.x / shadowMapSize.y;
	this->projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), aspectRatio, 0.1f, farPlane);
}

DirectX::XMMATRIX SpotLight::getLightViewProjection()
{
	DirectX::XMMATRIX viewMatrix = DirectX::XMMATRIX(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-position.x, -position.y, -position.z, 1.0f
	);
	HF::rotationMatrix(viewMatrix, rotation.x, rotation.y, 0);
	return -viewMatrix * projection;
}

const ViewProjConstantBufferData SpotLight::getLightViewProjectionBufferData()
{
	ViewProjConstantBufferData theReturn;
	memcpy(&theReturn.cameraPosition, &position, sizeof(DirectX::XMFLOAT3));
	theReturn.projectionMatrix = this->projection;
	DirectX::XMMATRIX viewMatrix = DirectX::XMMATRIX(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-position.x, -position.y, -position.z, 1.0f
	);
	HF::rotationMatrix(viewMatrix, rotation.x, rotation.y, 0);
	theReturn.viewMatrix = viewMatrix;

	return theReturn;
}

const DirectX::XMFLOAT3& SpotLight::getRotation()
{
	return rotation;
}

void SpotLight::setRotation(const DirectX::XMFLOAT3& rotation)
{
	this->rotation = rotation;
}
