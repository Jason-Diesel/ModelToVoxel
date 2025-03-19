#include "DirectionalLight.h"
#include "HelperFuncitons.h"

DirectionalLight::DirectionalLight(
	const DirectX::XMFLOAT3& position, 
	const DirectX::XMFLOAT3& rotation,
	const DirectX::XMUINT2& lightArea,
	DirectX::XMUINT2 shadowMapSize, 
	const float& farPlane
):
	Light(position), rotation(rotation)
{
	lightType = LightType::DirectionalLight_E;

	this->shadowMapSize = shadowMapSize;
	const float aspectRatio = shadowMapSize.x / shadowMapSize.y;
	this->projection = DirectX::XMMatrixOrthographicLH(lightArea.x, lightArea.y, 0.1f, farPlane);
}

DirectX::XMMATRIX DirectionalLight::getLightViewProjection()
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

const ViewProjConstantBufferData DirectionalLight::getLightViewProjectionBufferData()
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

const DirectX::XMFLOAT3& DirectionalLight::getRotation()
{
	return rotation;
}

void DirectionalLight::setRotation(const DirectX::XMFLOAT3& rotation)
{
	this->rotation = rotation;
}
