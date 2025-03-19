#include "Lights.h"

Light::Light(const DirectX::XMFLOAT3& position):
    position(position),
    color(DirectX::XMFLOAT4(1,1,1,1))
{
    this->position = position;
}

DirectX::XMMATRIX Light::getLightViewProjection()
{
    return DirectX::XMMatrixIdentity();
}

const ViewProjConstantBufferData Light::getLightViewProjectionBufferData()
{
    return ViewProjConstantBufferData();
}

const LightType Light::getLightType() const
{
    return this->lightType;
}

const DirectX::XMFLOAT3& Light::getPosition()
{
    return this->position;
}

const DirectX::XMFLOAT4& Light::getColor()
{
    return this->color;
}

const DirectX::XMFLOAT3& Light::getRotation()
{
    std::cout << "something wrong here, this shouldn't be called" << std::endl;
    return DirectX::XMFLOAT3();
}

void Light::setPosition(const DirectX::XMFLOAT3& position)
{
    this->position = position;
}

void Light::setColor(const DirectX::XMFLOAT4& color)
{
    this->color = color;
}

void Light::setColor(const DirectX::XMFLOAT3& color)
{
    memcpy(&this->color, &color, sizeof(DirectX::XMFLOAT3));
}

void Light::setRotation(const DirectX::XMFLOAT3& rotation)
{
    //Nothing here
}

const DirectX::XMUINT2& Light::getShadowMapSize()
{
    return this->shadowMapSize;
}

void Light::setShadowMapSizeInit(const DirectX::XMUINT2& sizes)
{
    this->shadowMapSize = sizes;
}

int32_t& Light::getShadowMapPos()
{
    return this->shadowMapPos;
}

void Light::setShadowSoftNess(const uint32_t shadowSoftNess)
{
    this->shadowSoftness = shadowSoftNess;
}

const uint32_t Light::getShadowMapSoftness() const
{
    return this->shadowSoftness;
}
