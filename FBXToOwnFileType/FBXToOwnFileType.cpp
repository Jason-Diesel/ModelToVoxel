// FBXToOwnFileType.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "FBXToOwnFileType.h"

int main()
{
    AssimpToOwnFileType("../Models/NewSponza_Main_glTF_003.gltf");
    AssimpToOwnFileType("../Models/NewSponza_Curtains_glTF.gltf");
}

void AssimpToOwnFileType(const std::string& fileName)
{
	struct stat buffer;
	if (!(stat(fileName.c_str(), &buffer) == 0)) {
		std::cout << "ERROR, cannot find file" << std::endl;
		return;
	}

	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(fileName.c_str(),
		aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_PreTransformVertices
	);

	if (!pScene) {
		std::wcout << L"couldn't find " << fileName.c_str() << " in directory: " << std::endl;
		printf("Error");
		return;
	}
	std::ofstream outputFile(fileName + ".OFT", std::ios::out | std::ios::binary | std::ios::app );

	if (!outputFile)
	{
		std::cout << "cannot Open File" << std::endl;
	}

	//How many meshes
	outputFile.write((char*)&pScene->mNumMeshes, sizeof(uint32_t));//change to uint32_t
	static uint32_t sn = 696969;

	std::vector<std::vector<Vertex>> vertecies;
	std::vector<std::vector<uint32_t>> indecies;
	std::vector<int> meshToMaterial;

	vertecies.resize(pScene->mNumMeshes);
	indecies.resize(pScene->mNumMeshes);
	meshToMaterial.resize(pScene->mNumMeshes);

	for (int i = 0; i < pScene->mNumMeshes; i++)
	{

		LoadMesh(vertecies[i], indecies[i], pScene->mMeshes[i]);

		const uint32_t NrOfVertecies = vertecies[i].size();
		const uint32_t NrOfIndecies = indecies[i].size();

		outputFile.write((char*)&NrOfVertecies, sizeof(uint32_t));
		outputFile.write((char*)vertecies[i].data(), NrOfVertecies * sizeof(Vertex));
		outputFile.write((char*)&NrOfIndecies, sizeof(uint32_t));
		outputFile.write((char*)indecies[i].data(), NrOfIndecies * sizeof(uint32_t));
		outputFile.write((char*)&pScene->mMeshes[i]->mMaterialIndex, sizeof(uint32_t));//change to uint32_t
	}

	std::vector<std::string> materialPaths;
	std::vector<Material> materials;

	LoadMaterialPaths(pScene, materials, materialPaths);

	//nr of Textures
	const uint32_t nrOfTextures = materialPaths.size();
	outputFile.write((char*)&nrOfTextures, sizeof(uint32_t));
	for (int i = 0; i < nrOfTextures; i++)
	{
		//Write File Path
		const uint32_t stringLenght = materialPaths[i].length();
		outputFile.write((char*)&stringLenght, sizeof(uint32_t));//How big the path is
		outputFile.write((char*)&materialPaths[i][0], stringLenght); //the actual materialPath
	}

	const uint32_t nrOfMaterials = materials.size();
	outputFile.write((char*)&nrOfMaterials, sizeof(uint32_t));
	for (uint32_t i = 0; i < nrOfMaterials; i++)
	{
		//Ka, Kd, Ks, Ns
		outputFile.write((char*)&materials[i].ka, sizeof(Vec3));
		outputFile.write((char*)&materials[i].kd, sizeof(Vec3));
		outputFile.write((char*)&materials[i].ks, sizeof(Vec3));
		outputFile.write((char*)&materials[i].ns, sizeof(Vec3));

		//NrOfTextures
		const uint32_t nrOfTextures = materials[i].textureIndex.size();
		outputFile.write((char*)&nrOfTextures, sizeof(uint32_t));
		//MaterialFlag
		outputFile.write((char*)&materials[i].materialFlags, sizeof(int32_t));
		//MaterialIndex In order
		if (nrOfTextures > 0)
		{
			outputFile.write((char*)materials[i].textureIndex.data(), sizeof(int32_t) * nrOfTextures);
		}
	}
	outputFile.close();
}

void LoadMesh(
	std::vector<Vertex>& vertecies, 
	std::vector<uint32_t>& indecies,
	aiMesh* pMesh
)
{
	vertecies.reserve(pMesh->mNumVertices);
	indecies.reserve(pMesh->mNumFaces * 3);

	for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
	{
		Vertex vert;
		vert.pos.x = pMesh->mVertices[i].x;
		vert.pos.y = pMesh->mVertices[i].y;
		vert.pos.z = pMesh->mVertices[i].z;

		vert.normal.x = pMesh->mNormals[i].x;
		vert.normal.y = pMesh->mNormals[i].y;
		vert.normal.z = pMesh->mNormals[i].z;

		vert.tangent.x = pMesh->mTangents[i].x;
		vert.tangent.y = pMesh->mTangents[i].y;
		vert.tangent.z = pMesh->mTangents[i].z;

		if (pMesh->mTextureCoords[0] != nullptr)
		{
			vert.uv.x = pMesh->mTextureCoords[0][i].x;
			vert.uv.y = pMesh->mTextureCoords[0][i].y;
		}
		else {
			vert.uv.x = 0;
			vert.uv.y = 0;
		}
		vertecies.push_back(vert);
	}

	for (uint32_t i = 0; i < pMesh->mNumFaces; i++) {
		indecies.push_back(pMesh->mFaces[i].mIndices[0]);
		indecies.push_back(pMesh->mFaces[i].mIndices[1]);
		indecies.push_back(pMesh->mFaces[i].mIndices[2]);
	}
}

void LoadMaterialPaths(
	const aiScene* pScene, 
	std::vector<Material>& material,
	std::vector<std::string>& texturePaths
)
{
	material.resize(pScene->mNumMaterials);
	texturePaths.reserve(pScene->mNumMaterials);

	const aiTextureType allTextureSupportedTypes[] = {
			aiTextureType_DIFFUSE,
			aiTextureType_NORMALS,
			//aiTextureType_DISPLACEMENT,
			//aiTextureType_LIGHTMAP
	};

	for (uint32_t m = 0; m < pScene->mNumMaterials; m++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[m];

		bool GotTypes[_countof(allTextureSupportedTypes)] = { false };
		int nrOfTextures = 0;
		for (int i = 0; i < _countof(allTextureSupportedTypes); i++)
		{
			if (pMaterial->GetTextureCount(allTextureSupportedTypes[i]) > 0)
			{
				GotTypes[i] = true;
				nrOfTextures++;
			}
		}
		if (nrOfTextures == 0) { continue; }

		{
			aiColor4D ka, kd, ks, ns;
			if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &ka))
				memcpy(&material[m].ka, &ka, sizeof(Vec3));
			if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &kd))
				memcpy(&material[m].kd, &kd, sizeof(Vec3));
			if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_SPECULAR, &ks))
				memcpy(&material[m].ks, &ks, sizeof(Vec3));
			if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_SHININESS, &ns))
				memcpy(&material[m].ns, &ns, sizeof(Vec3));
		}

		for (int i = 0; i < _countof(allTextureSupportedTypes); i++)
		{
			if (!GotTypes[i]) { continue; }
			aiString path;
			if (pMaterial->GetTexture(allTextureSupportedTypes[i], 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				texturePaths.push_back(path.C_Str());
			}
			else {
				texturePaths.push_back("Error");
			}
			material[m].materialFlags = (MaterialFlags)(material[m].materialFlags | MaterialFlagsArray[i]);
			material[m].textureIndex.push_back(texturePaths.size() - 1);
		}
	}
}
