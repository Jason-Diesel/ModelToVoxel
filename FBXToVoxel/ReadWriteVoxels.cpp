#include "ReadWriteVoxels.h"
#include "filesystem"

void WriteVoxelToFile(const DirectX::XMUINT3& sizes, Voxel* voxelGrid, const std::string& fileName)
{
	struct stat buffer;
	if ((stat(fileName.c_str(), &buffer) == 0)) {
		std::filesystem::remove(fileName);
	}
	std::ofstream file;
	file.open(fileName, std::ios::out | std::ios::binary | std::ios::app);
	if(!file.is_open()){
		return;
	}
	file.write((char*)&sizes, sizeof(DirectX::XMUINT3));
	const uint32_t DataSize = sizes.x * sizes.y * sizes.z * sizeof(Voxel);
	file.write((char*)voxelGrid, DataSize);

	file.close();
}

void ReadVoxelFromFile(DirectX::XMUINT3& sizes, Voxel*& voxelGrid, const std::string& fileName)
{
	struct stat buffer;
	if (!(stat(fileName.c_str(), &buffer) == 0)) {
		return;
	}
	std::ifstream file;
	file.open(fileName, std::ios::out | std::ios::binary | std::ios::app);
	if (!file.is_open()) {
		return;
	}
	file.read((char*)&sizes, sizeof(DirectX::XMUINT3));
	const uint32_t NumberOfVoxels = sizes.x * sizes.y * sizes.z;
	const uint32_t DataSize = NumberOfVoxels * sizeof(Voxel);
	voxelGrid = new Voxel[NumberOfVoxels];
	file.read((char*)voxelGrid, DataSize);

	file.close();
}
