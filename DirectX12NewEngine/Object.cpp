#include "Object.h"

Object::Object(Graphics* gfx):
	scal(1,1,1),
	rotation(0,0,0),
	position(0,0,0)
{
	transformBuffer = CreateConstantBuffer<DirectX::XMMATRIX>(gfx);
}

Object::Object() :
	scal(1, 1, 1),
	rotation(0, 0, 0),
	position(0, 0, 0)
{
}

Object::~Object()
{
	//for (auto& it : components) {
	//	// Do stuff		
	//	if (it.second[0]->deleteInObject) {
	//		for (int i = 0; i < it.second.size(); i++)
	//		{
	//			delete it.second[i];
	//		}
	//	}
	//}
}

void Object::setPosition(const DirectX::XMFLOAT3& position)
{
	this->position = position;
}

void Object::setRotation(const DirectX::XMFLOAT3& rotation)
{
	this->rotation = rotation;
}

void Object::setScale(const DirectX::XMFLOAT3& scale)
{
	this->scal = scale;
}

void Object::move(const DirectX::XMFLOAT3& direction)
{
	this->position.x += direction.x;
	this->position.y += direction.y;
	this->position.z += direction.z;
}

void Object::rotate(const DirectX::XMFLOAT3& angles)
{
	rotation.x += angles.x;
	rotation.y += angles.y;
	rotation.z += angles.z;
}

void Object::scale(const DirectX::XMFLOAT3& scale)
{
	scal.x *= scale.x;
	scal.y *= scale.y;
	scal.z *= scale.z;
}

void Object::scale(const float& scale)
{
	scal.x *= scale;
	scal.y *= scale;
	scal.z *= scale;
}

DirectX::XMFLOAT3& Object::getPosition()
{
	return position;
}

DirectX::XMFLOAT3& Object::getRotation()
{
	return rotation;
}

DirectX::XMFLOAT3& Object::getScale()
{
	return scal;
}

void Object::setConstantBuffer(Graphics* gfx)
{
	updateConstantBuffer(CreateTransformationMatrix(), transformBuffer);
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(1, transformBuffer.constantBuffer->GetGPUVirtualAddress());
}

DirectX::XMMATRIX Object::CreateTransformationMatrix()
{
	return DirectX::XMMatrixScaling(scal.x, scal.y, scal.z) *
		DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
		DirectX::XMMatrixTranslation(position.x, position.y, position.z);
}