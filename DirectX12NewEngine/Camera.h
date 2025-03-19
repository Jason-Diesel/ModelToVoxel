#pragma once
#include "CommonHeaders.h"
#include "ConstantBuffers.h"

class Camera {
public:
	Camera();
	~Camera();
	void init();
	void update();
	//setters
	void setPosition(const DirectX::XMFLOAT3& position);
	void setRotation(const DirectX::XMFLOAT3& rotation);
	void move(const DirectX::XMFLOAT3& direction);
	void rotate(const DirectX::XMFLOAT3& roation);

	void addPositionBasedOnDirection(DirectX::XMFLOAT3 direction, float speed = 1);

	//getters
	DirectX::XMFLOAT3 getPostion() const;
	DirectX::XMFLOAT3 getRotation() const;
	DirectX::XMMATRIX getCameraViewMatrix() const;
	DirectX::XMMATRIX getProjectionMatrix() const;
	ViewProjConstantBufferData getViewProjConstantBufferData();

	void createNewProjectionMatrix(float fov = 45, float windowWidth = 1920, float windowHeight = 1080, float nearPlane = 0.1, float farPlane = 10000.0f);
	void createNewOrthagonalMatrix(float width, float height, float nearPlane = 0.1, float farPlane = 10000.0f);

	//DEBUG
	void setLastFloat(const float data);

private:
	DirectX::XMFLOAT3 postion;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMMATRIX Projection;

	ViewProjConstantBufferData viewProjectionData;
};