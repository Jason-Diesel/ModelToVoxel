#include "PointLight.h"

PointLight::PointLight(const DirectX::XMFLOAT3& position):
	Light(position)
{
	lightType = LightType::PointLight_E;
	this->shadowMapPos = -1;
}
