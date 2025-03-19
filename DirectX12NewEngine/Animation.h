#pragma once
#include "CommonHeaders.h"
#include <vector>
#include <unordered_map>

struct KeyFrames
{
	std::vector<float> positionTimestamps;
	std::vector<float> rotationTimestamps;
	std::vector<float> scaleTimestamps;

	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT4> rotations;
	std::vector<DirectX::XMFLOAT3> scale;
};

struct SkeletalAnimation
{
	float lenght = 0;
	float tick = 1.0f;
	std::unordered_map<std::string, KeyFrames> keyframes;//Later we don't use std::string
};

struct Bone {
	std::string name;
	int parentIndex = -1;
	int id = -1;

	DirectX::XMMATRIX inverseBindPoseMatrix;
	DirectX::XMMATRIX FinalTransformation;
};

struct buildBone {
	std::vector<buildBone> childJoints;
	std::string name;
	int id = -1;
	DirectX::XMMATRIX inverseBindPoseMatrix;
	DirectX::XMMATRIX boneMatrix;

};