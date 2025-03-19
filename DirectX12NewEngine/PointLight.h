#pragma once
#include "Lights.h"

class PointLight : public Light
{
public:
	PointLight(const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(0,0,0));
};