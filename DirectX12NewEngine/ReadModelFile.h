#pragma once
#include "Model.h"
#include "Vertex.h"
#include "Animation.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "VertexBufferCreator.h"
#include "readImage.h"
#include "ResourceManager.h"
#include <fstream>

class FileReader {
public:
	FileReader(Graphics* gfx, ResourceManager* rm);

	Model* readModel(const std::string& fileName);
	SkeletalAnimation* readAnimation(const std::string& animationFile);
	void loadMaterials(std::vector<Material*>& textures, const aiScene* pScene);
	//TextureViewClass* createTexture(const std::string& filePath);

private:
	Model* readOTFFiles(const std::string& fileName);
	Model* readAssimpFiles(const std::string& fileName);

	SkeletalAnimation* readOTFAnimation(const std::string& fileName);
	SkeletalAnimation* readASSIMPAnimation(const std::string& fileName);
	bool readSkeleton(buildBone& joint, aiNode* node, std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>>& offsetMatrices);
	void buildSkeleton(std::vector<Bone>& skeleton, const buildBone& rootBone, int parentID);

	void loadMesh(aiMesh* pMesh, Mesh& mesh, std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>>& boneInfo, const bool bones = false);
	void loadBoneDataToVerteies(std::vector<SkeletalVertex>& skeletalVerticies, const aiMesh* pMesh, std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>>& offsetMatrices);
	//void loadMeshBones(std::vector<SkeletalVertex>& skeletalVerticies, const aiMesh* pMesh, std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>>& offsetMatrices);
	Graphics* gfx;
	ResourceManager* rm;

	const uint32_t MaterialFlagArray[4] = { 1, 2, 4, 8 };
	//have a resource manager here later
};





