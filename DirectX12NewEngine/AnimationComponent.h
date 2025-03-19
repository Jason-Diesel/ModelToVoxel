#pragma once
#include "Components.h"
#include "Animation.h"
#include "ConstantBuffers.h"
#include "Model.h"

class AnimationComponent : public Component
{
public:
	AnimationComponent(Graphics* gfx);
	~AnimationComponent();

	void setAnimation(SkeletalAnimation* animation);

	void update(float dt);
	void setPose(AnimatedModel* animationModel);
	ConstantBuffer skeletalConstantBuffer;
private:
	SkeletalAnimation* currentAnimation;
private:
	float time = 0;
	float lastTime = 0;
};