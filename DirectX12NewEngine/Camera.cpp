
#include "Camera.h"
#include "HelperFuncitons.h"

Camera::Camera()
{
	this->postion = DirectX::XMFLOAT3(0, 0, -1);
	this->rotation = DirectX::XMFLOAT3(0, 0, 0);
	this->createNewProjectionMatrix(45, 1920,1080, 0.1f, 10000.0f);
	//this->createNewProjectionMatrix(45, 2000, 2000, 0.1f, 10000.0f);
}

Camera::~Camera()
{
}

void Camera::init()
{
}

void Camera::update()
{
	viewProjectionData.viewMatrix = getCameraViewMatrix();
	memcpy(&viewProjectionData.cameraPosition, &postion, sizeof(DirectX::XMFLOAT3));
}

void Camera::setPosition(const DirectX::XMFLOAT3& position)
{
	this->postion = position;
}

void Camera::setRotation(const DirectX::XMFLOAT3& rotation)
{
	this->rotation = rotation;
}

void Camera::move(const DirectX::XMFLOAT3& direction)
{
	this->postion = HF::add(this->postion, direction);
}

void Camera::rotate(const DirectX::XMFLOAT3& rotation)
{
	this->rotation = HF::add(this->rotation, rotation);
}

void Camera::addPositionBasedOnDirection(DirectX::XMFLOAT3 direction, float speed)
{
	DirectX::XMStoreFloat3(&direction, DirectX::XMVector3Transform(
		DirectX::XMLoadFloat3(&direction),
		DirectX::XMMatrixRotationRollPitchYaw(rotation.y, rotation.x, 0) *
		DirectX::XMMatrixScaling(1, 1, 1)//this line is not neccessary but I am afraid to break things
	));
	DirectX::XMFLOAT2 trans(direction.x, direction.z);
	DirectX::XMVECTOR XMtrans = DirectX::XMVector2Normalize(DirectX::XMLoadFloat2(&trans));
	DirectX::XMStoreFloat2(&trans, XMtrans);
	postion.x -= trans.x * speed;
	postion.z -= trans.y * speed;
}

DirectX::XMFLOAT3 Camera::getPostion() const
{
	return this->postion;
}

DirectX::XMFLOAT3 Camera::getRotation() const
{
	return this->rotation;
}

DirectX::XMMATRIX Camera::getCameraViewMatrix() const
{
	DirectX::XMMATRIX viewMatrix = DirectX::XMMATRIX(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-postion.x, -postion.y, -postion.z, 1.0f
	);
	HF::rotationMatrix(viewMatrix, rotation.x, rotation.y, 0);
	return viewMatrix;
}

DirectX::XMMATRIX Camera::getProjectionMatrix() const
{
	return this->Projection;
}

ViewProjConstantBufferData Camera::getViewProjConstantBufferData()
{
	return this->viewProjectionData;
}

void Camera::createNewProjectionMatrix(float fov, float windowWidth, float windowHeight, float nearPlane, float farPlane)
{
	const float aspectRatio = windowWidth / windowHeight;
	this->Projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), aspectRatio, nearPlane, farPlane);
	viewProjectionData.projectionMatrix = Projection;
}

void Camera::createNewOrthagonalMatrix(float width, float height, float nearPlane, float farPlane)
{
	this->Projection = DirectX::XMMatrixOrthographicLH(width, height, nearPlane, farPlane);
	viewProjectionData.projectionMatrix = Projection;
}

void Camera::setLastFloat(const float data)
{
	viewProjectionData.cameraPosition.w = data;
}
