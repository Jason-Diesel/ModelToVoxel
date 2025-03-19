#pragma once
#include "Mesh.h"
#include "Components.h"
#include <vector>

struct Model : public Component{
	Model();
	virtual ~Model();
	Mesh* subMeshes;
	uint32_t nrOfSubMeshes;
	//SHALL HAVE A ANIMATION SLOT AS WELL
	enum {
		_Model,
		_AnimatedModel
	}ModelType;
};

struct AnimatedModel : public Model {
	AnimatedModel(Graphics* gfx);
	std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>> boneInfo = {};
	std::vector<Bone> skeleton;
};