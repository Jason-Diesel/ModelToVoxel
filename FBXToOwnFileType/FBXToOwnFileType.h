#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "OFTFile.h"
#include <fstream>
#include <vector>

struct Vec2 {
	float x;
	float y;
};

struct Vec3 {
	float x = 0;
	float y = 0;
	float z = 0;
};
//TODO : a struct for bones

struct Vertex {
	Vec3 pos;
	Vec2 uv;
	Vec3 normal;
	Vec3 tangent;
	//another one for tangent?
};

struct Material {
	std::vector<int32_t> textureIndex;//already in order
	MaterialFlags materialFlags = MaterialFlags::None;
	Vec3 ka;
	Vec3 kd;
	Vec3 ks;
	Vec3 ns;
};

#ifdef _DEBUG

struct{
	uint32_t nrOfMeshes[2];
	std::vector<uint32_t> nrOfVertecies[2];
	std::vector<uint32_t> nrOfIndecies[2];
}TestStruct;

#else

#define TestStruct //

#endif

void AssimpToOwnFileType(const std::string& fileName);
void LoadMesh(
	std::vector<Vertex>& vertecies,
	std::vector<uint32_t>& indecies,
	aiMesh* pMesh
);
void LoadMaterialPaths(
	const aiScene* pScene, 
	std::vector<Material>& material,
	std::vector<std::string>& texturePaths
);

void readOFT(const std::string& fileName);