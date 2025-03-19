#pragma once
#include "VoxelAndVertecies.h"
#include <fstream>


void WriteVoxelToFile(
	const DirectX::XMUINT3& sizes,
	Voxel* voxelGrid,
	const std::string& fileName
	);

void ReadVoxelFromFile(
	DirectX::XMUINT3& sizes,
	Voxel*& voxelGrid,
	const std::string& fileName
);