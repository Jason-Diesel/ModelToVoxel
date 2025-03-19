#include "ReadModelFile.h"
#include "HelperFuncitons.h"

static const std::string ExtraNamingModel = "Model_Resource_";
static const std::string ExtraNamingAnimation = "Animation_Resource_";
static const std::string ExtraNamingMaterial = "Material_Resource_";
static const std::string ExtraNamingTexture = "Texture_Resource_";

DirectX::XMMATRIX ConvertToXMMATRIX(const aiMatrix4x4& aiMat)
{
	return DirectX::XMMatrixSet(
		aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
		aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
		aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
		aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
	);
}

FileReader::FileReader(Graphics* gfx, ResourceManager* rm):
	gfx(gfx), rm(rm)
{}

Model* FileReader::readModel(const std::string& fileName)
{
	Model* theReturnModel;
	theReturnModel = rm->getResource<Model>(ExtraNamingModel + fileName);
	if (theReturnModel != nullptr)
	{
		return theReturnModel;
	}
	
	struct stat buffer;
	if (!(stat(fileName.c_str(), &buffer) == 0)) {
		return nullptr;
	}
	
	if (fileName.substr(fileName.size() - 4) == ".OFT")
	{
		theReturnModel = readOTFFiles(fileName);
	}
	else 
	{
		theReturnModel = readAssimpFiles(fileName);
	}
	return theReturnModel;
	
}

SkeletalAnimation* FileReader::readAnimation(const std::string& animationFile)
{
	SkeletalAnimation* theReturnAnimation;
	theReturnAnimation = rm->getResource<SkeletalAnimation>(ExtraNamingAnimation + animationFile);
	if (theReturnAnimation != nullptr)
	{
		return theReturnAnimation;
	}

	struct stat buffer;
	if (!(stat(animationFile.c_str(), &buffer) == 0)) {
		return nullptr;
	}

	if (animationFile.substr(animationFile.size() - 4) == ".OFT")
	{
		//Not yet supported
		theReturnAnimation = readOTFAnimation(animationFile);
	}
	else
	{
		theReturnAnimation = readASSIMPAnimation(animationFile);
	}
	//ADD EMPTY ANIMATION FOR EMPTY JOINTS

	rm->addResource(theReturnAnimation, ExtraNamingAnimation + animationFile);
	return theReturnAnimation;
}

void FileReader::loadMaterials(std::vector<Material*>& materials, const aiScene* pScene)
{
	materials.resize(pScene->mNumMaterials);
	//TODO : REMAKE THIS CODE A LITTLE BIT
	for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
	{
		materials[i] = new Material();
	}

	for (uint32_t m = 0; m < pScene->mNumMaterials; m++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[m];
		
		//const aiTextureType allTextureSupportedTypes[] = {
		//	aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_AMBIENT
		//};
		const aiTextureType allTextureSupportedTypes[] = {
			aiTextureType_DIFFUSE, aiTextureType_NORMALS, aiTextureType_DISPLACEMENT
		};
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
		
		//materials[m]->init(nrOfTextures);
		static DefaultMaterialData defmatData;

		aiColor4D ka, kd, ks, ns;
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &ka))
			memcpy(&defmatData.ka, &ka, sizeof(DirectX::XMFLOAT3));
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &kd))
			memcpy(&defmatData.kd, &kd, sizeof(DirectX::XMFLOAT3));
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_SPECULAR, &ks))
			memcpy(&defmatData.ks, &ks, sizeof(DirectX::XMFLOAT3));
		if (AI_SUCCESS == aiGetMaterialColor(pMaterial, AI_MATKEY_SHININESS, &ns))
			memcpy(&defmatData.ns, &ns, sizeof(DirectX::XMFLOAT3));

		materials[m]->init(gfx, nrOfTextures, sizeof(DefaultMaterialData));
		int textureIndex = 0;
		uint32_t materialFlag = 0;
		for (int i = 0; i < _countof(allTextureSupportedTypes); i++)
		{
			if (!GotTypes[i]) { continue; }
			aiString path;
			if (pMaterial->GetTexture(allTextureSupportedTypes[i], 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) 
			{
				materialFlag = materialFlag | MaterialFlagArray[i];
				TextureViewClass* tex = rm->getResource<TextureViewClass>(ExtraNamingTexture + path.C_Str());
				if (tex == nullptr)
				{
					//load diffuse Texture
					std::string nPath;
					if (std::string(path.C_Str()).substr(0, 2) == "C:")
					{
						nPath = std::string(path.C_Str());
					}
					else {
						nPath = "../" + std::string(path.C_Str());
					}
					
					tex = createTexture(nPath, rm, gfx);
					if (tex == nullptr)
					{
						materials[m]->addTexture(textureIndex++, rm->getResource<TextureViewClass>(ExtraNamingTexture + "../textures/7fJYp.png"), gfx);
						return;
					}
					materials[m]->addTexture(textureIndex++, tex, gfx);
				}
				else {
					materials[m]->addTexture(textureIndex++, tex, gfx);
				}
			}
		}

		materials[m]->finalize(materialFlag);
		materials[m]->updateMaterialData(defmatData);
	}
}

Model* FileReader::readOTFFiles(const std::string& fileName)
{
	uint32_t sixnine;
	Model* theReturnModel = new Model();

	std::ifstream fileReader(fileName, std::ios::out | std::ios::binary | std::ios::app);
	
	if (!fileReader)
	{
		std::cout << "error cannot find file" << std::endl;
		return nullptr;
	}
	
	fileReader.read((char*)&theReturnModel->nrOfSubMeshes, sizeof(uint32_t));
	
	const uint32_t nrOfSubMeshes = theReturnModel->nrOfSubMeshes;
	theReturnModel->subMeshes = new Mesh[nrOfSubMeshes];
	std::vector<int32_t> materialIndexPerMesh(nrOfSubMeshes);
	
	for (uint32_t i = 0; i < nrOfSubMeshes; i++)
	{
	
		fileReader.read((char*)&theReturnModel->subMeshes[i].nrOfVertecies, sizeof(uint32_t));
	
		//Vertecies
		const uint32_t nrOfVertecies = theReturnModel->subMeshes[i].nrOfVertecies;
		std::vector<OFTVertex> OTFVertecies(nrOfVertecies);
		std::vector<Vertex> vertecies(nrOfVertecies);
	
		fileReader.read((char*)&OTFVertecies[0], sizeof(OFTVertex) * nrOfVertecies);
	
		for (uint32_t i = 0; i < nrOfVertecies; i++)
		{
			memcpy(&vertecies[i], &OTFVertecies[i], sizeof(OFTVertex));
		}
		for (uint32_t i = 0; i < nrOfVertecies; i++)
		{
			DirectX::FXMVECTOR normal = DirectX::XMLoadFloat3(&vertecies[i].normal);
			DirectX::FXMVECTOR tangent = DirectX::XMLoadFloat3(&vertecies[i].tangent);
			DirectX::XMStoreFloat3(&vertecies[i].bitangent, DirectX::XMVector3Cross(normal, tangent));
		}
	
		theReturnModel->subMeshes[i].vertexBuffer = createVertexBuffer(
			vertecies,
			gfx->getDevice(),
			gfx->getCommandList(),//unsure If I should I have a number here?
			gfx->getCommandAllocator(),//unsure If I should I have a number here?
			gfx->getCommandQueue(),
			gfx->getFenceEvent(),
			gfx->getFence(),
			gfx->getFenceValue()
		);
	
		fileReader.read((char*)&theReturnModel->subMeshes[i].nrOfIndecies, sizeof(uint32_t));
	
		const uint32_t nrOfIndecies = theReturnModel->subMeshes[i].nrOfIndecies;
		std::vector<uint32_t> indecies(nrOfIndecies);
		fileReader.read((char*)&indecies[0], sizeof(uint32_t) * nrOfIndecies);
		theReturnModel->subMeshes[i].indeciesBuffer = createIndeciesBuffer(
			indecies,
			gfx->getDevice(),
			gfx->getCommandList(),//unsure If I should I have a number here?
			gfx->getCommandAllocator(),//unsure If I should I have a number here?
			gfx->getCommandQueue(),
			gfx->getFenceEvent(),
			gfx->getFence(),
			gfx->getFenceValue()
		);
	
		fileReader.read((char*)&materialIndexPerMesh[i], sizeof(uint32_t));
	
		theReturnModel->subMeshes[i].vertexBufferView.BufferLocation = theReturnModel->subMeshes[i].vertexBuffer->GetGPUVirtualAddress();
		theReturnModel->subMeshes[i].vertexBufferView.SizeInBytes = nrOfVertecies * sizeof(Vertex);
		theReturnModel->subMeshes[i].vertexBufferView.StrideInBytes = sizeof(Vertex);
	
		theReturnModel->subMeshes[i].indeciesBufferView.BufferLocation = theReturnModel->subMeshes[i].indeciesBuffer->GetGPUVirtualAddress();
		theReturnModel->subMeshes[i].indeciesBufferView.SizeInBytes = (UINT)(nrOfIndecies * sizeof(uint32_t));
		theReturnModel->subMeshes[i].indeciesBufferView.Format = DXGI_FORMAT_R32_UINT;
	}
	
	std::vector<TextureViewClass*> textures;
	{
		uint32_t nrOfTextures;
		fileReader.read((char*)&nrOfTextures, sizeof(uint32_t));
		textures.resize(nrOfTextures);
	
		for (int i = 0; i < nrOfTextures; i++)
		{
			uint32_t stringSize;
			fileReader.read((char*)&stringSize, sizeof(uint32_t));
			std::string textureFilePath;
			textureFilePath.resize(stringSize);
			fileReader.read((char*)&textureFilePath[0], stringSize);
	
			textures[i] = rm->getResource<TextureViewClass>(ExtraNamingTexture + textureFilePath);
			if (textures[i] == nullptr)
			{
				if (textureFilePath.substr(0, 2) != "C:")
				{
					textureFilePath = "../" + textureFilePath;
				}
				textures[i] = createTexture(textureFilePath, rm, gfx);
			}
		}
	}
	
	uint32_t nrOfMaterials;
	fileReader.read((char*)&nrOfMaterials, sizeof(uint32_t));
	
	DirectX::XMFLOAT3 kakdksns[4];
	
	std::vector<Material*> materials;
	
	for (uint32_t i = 0; i < nrOfMaterials; i++)
	{
		DefaultMaterialData defMatData;
		fileReader.read((char*)&kakdksns[0], sizeof(DirectX::XMFLOAT3) * 4);
	
		defMatData.ka = DirectX::XMFLOAT4(kakdksns[0].x, kakdksns[0].y, kakdksns[0].z, 0);
		defMatData.kd = DirectX::XMFLOAT4(kakdksns[1].x, kakdksns[1].y, kakdksns[1].z, 0);
		defMatData.ks = DirectX::XMFLOAT4(kakdksns[2].x, kakdksns[2].y, kakdksns[2].z, 0);
		defMatData.ns = DirectX::XMFLOAT4(kakdksns[3].x, kakdksns[3].y, kakdksns[3].z, 0);
		
		defMatData.ka = DirectX::XMFLOAT4(0.4, 0.4, 0.4, 0);

		uint32_t nrOfTextures;
		int32_t materialFlag;
		std::vector<int32_t> TextureIndex;
		fileReader.read((char*)&nrOfTextures, sizeof(uint32_t));
		fileReader.read((char*)&materialFlag, sizeof(int32_t));
		TextureIndex.resize(nrOfTextures);
		if (nrOfTextures > 0)
		{
			fileReader.read((char*)&TextureIndex[0], sizeof(int32_t) * nrOfTextures);
	
			Material* mat = new Material();
			mat->init(gfx, nrOfTextures, sizeof(DefaultMaterialData));
			for (uint16_t t = 0; t < nrOfTextures; t++)
			{
				mat->addTexture(t, textures[TextureIndex[t]], gfx);
			}
			mat->finalize(materialFlag);
			mat->updateMaterialData(defMatData);
			static int a = 0;
			rm->addResource(mat, "Material_" + std::to_string(a++));
			
			materials.push_back(mat);
		}
		else 
		{
			materials.push_back(rm->getResource<Material>("_Def_Material"));
		}
	}
	
	for (int i = 0; i < materialIndexPerMesh.size(); i++)
	{
		theReturnModel->subMeshes[i].material = materials[materialIndexPerMesh[i]];
	}
	fileReader.close();
	rm->addResource(theReturnModel, ExtraNamingModel + fileName);
	return theReturnModel;
}

Model* FileReader::readAssimpFiles(const std::string& fileName)
{
	Model* theReturnModel;
	//FOR SOME FUCKING REASON I GET A MEMORY LEAK AT READFILE, SO WE ARE GOING TO CREATE OUR OWN FILE TYPE
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(fileName.c_str(),
		aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs
	);

	if (!pScene) {
		std::wcout << L"couldn't find " << fileName.c_str() << " in directory: " << HF::getCurrentDirectory() << std::endl;
		printf("Error");
		return nullptr;
	}
	
	if (pScene->HasAnimations())
	{
		theReturnModel = new AnimatedModel(gfx);
	}
	else 
	{
		theReturnModel = new Model();
	}

	theReturnModel->subMeshes = new Mesh[pScene->mNumMeshes];
	
	theReturnModel->nrOfSubMeshes = pScene->mNumMeshes;

	std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>> boneInfo = {};
	for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
	{
		loadMesh(pScene->mMeshes[i], theReturnModel->subMeshes[i], boneInfo, pScene->mMeshes[i]->HasBones());
	}
	if (pScene->HasAnimations())
	{
		buildBone rootBone;
		readSkeleton(rootBone, pScene->mRootNode, boneInfo);
		((AnimatedModel*)theReturnModel)->skeleton.resize(boneInfo.size());
		buildSkeleton(((AnimatedModel*)theReturnModel)->skeleton, rootBone, -1);
	}

	std::vector<Material*> materials;
	loadMaterials(materials, pScene);

	for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
	{
		if (materials[pScene->mMeshes[i]->mMaterialIndex]->getNrOfTextures() < 1)
		{
#if (ANYMATERIALBEFOREDEFAULTMATERIAL)

			//JUST GIVE ANY MATERIAL
			bool done = false;
			for (int m = 0; m < materials.size() && !done; m++)
			{
				if (materials[m]->getNrOfTextures() != 0)
				{
					theReturnModel->subMeshes[i].material = materials[m];
					done = true;
				}
			}
			if (!done)
			{
				theReturnModel->subMeshes[i].material = rm->getResource<Material>("_Def_Material");
				std::cout << "Warning : Cannot find material for subMesh" << std::endl;
			}
#else
			theReturnModel->subMeshes[i].material = rm->getResource<Material>("_Def_Material");
			std::cout << "Warning : Cannot find material for subMesh" << std::endl;
#endif	
		}
		else {
			theReturnModel->subMeshes[i].material = materials[pScene->mMeshes[i]->mMaterialIndex];
		}
	}

	rm->addResource(theReturnModel, ExtraNamingModel + fileName);

	return theReturnModel;
}

SkeletalAnimation* FileReader::readOTFAnimation(const std::string& fileName)
{
	return nullptr;
}

SkeletalAnimation* FileReader::readASSIMPAnimation(const std::string& fileName)
{
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(fileName.c_str(), aiProcessPreset_TargetRealtime_Fast);

	if (!pScene) {
		std::wcout << L"couldn't find " << fileName.c_str() << " in directory: " << HF::getCurrentDirectory() << std::endl;
		printf("Error");
		return nullptr;
	}

	if (!pScene->HasAnimations())
	{
		printf("Error couldn't find animation");
		return nullptr;
	}

	SkeletalAnimation* returnSkeletalAnimation = new SkeletalAnimation();


	aiAnimation* anim = pScene->mAnimations[0];

	returnSkeletalAnimation->tick = (float)anim->mTicksPerSecond;
	returnSkeletalAnimation->lenght = anim->mDuration;
	returnSkeletalAnimation->keyframes = {};
	for (uint32_t i = 0; i < anim->mNumChannels; i++)
	{
		aiNodeAnim* channel = anim->mChannels[i];
		KeyFrames track;
		const int CurrentSizeOfTrackPosition = track.positions.size();
		track.positions.resize(track.positions.size() + channel->mNumPositionKeys);
		track.positionTimestamps.resize(track.positions.size() + channel->mNumPositionKeys);
		for (uint32_t j = 0; j < channel->mNumPositionKeys; j++)
		{
			track.positionTimestamps[CurrentSizeOfTrackPosition + j] = (float)channel->mPositionKeys[j].mTime;
			memcpy(&track.positions[CurrentSizeOfTrackPosition + j], &channel->mPositionKeys[j].mValue, sizeof(DirectX::XMFLOAT3));
		}

		const int CurrentSizeOfTrackRotation = track.rotations.size();
		track.rotations.resize(track.rotations.size() + channel->mNumRotationKeys);
		track.rotationTimestamps.resize(track.rotations.size() + channel->mNumRotationKeys);
		for (uint32_t j = 0; j < channel->mNumRotationKeys; j++) {
			track.rotationTimestamps[CurrentSizeOfTrackRotation + j] = (float)channel->mRotationKeys[j].mTime;
			track.rotations[CurrentSizeOfTrackRotation + j].x = channel->mRotationKeys[j].mValue.x;
			track.rotations[CurrentSizeOfTrackRotation + j].y = channel->mRotationKeys[j].mValue.y;
			track.rotations[CurrentSizeOfTrackRotation + j].z = channel->mRotationKeys[j].mValue.z;
			track.rotations[CurrentSizeOfTrackRotation + j].w = channel->mRotationKeys[j].mValue.w;
		}

		const int CurrentSizeOfTrackScale = track.scale.size();
		track.scale.resize(track.scale.size() + channel->mNumPositionKeys);
		track.scaleTimestamps.resize(track.scale.size() + channel->mNumPositionKeys);
		for (uint32_t j = 0; j < channel->mNumScalingKeys; j++)
		{
			track.scaleTimestamps[CurrentSizeOfTrackScale + j] = (float)channel->mScalingKeys[j].mTime;
			memcpy(&track.scale[CurrentSizeOfTrackScale + j], &channel->mScalingKeys[j].mValue, sizeof(DirectX::XMFLOAT3));
		}
		returnSkeletalAnimation->keyframes.insert(std::pair<std::string, KeyFrames>(channel->mNodeName.C_Str(), track));
	}

	return returnSkeletalAnimation;
}

bool FileReader::readSkeleton(buildBone& joint, aiNode* node, std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>>& offsetMatrices)
{
	if (offsetMatrices.find(node->mName.C_Str()) != offsetMatrices.end()) { // if node is actually a bone
		joint.name = node->mName.C_Str();
		joint.id = offsetMatrices[joint.name].first;
		DirectX::XMMATRIX off = offsetMatrices[joint.name].second;
		joint.inverseBindPoseMatrix = DirectX::XMMatrixTranspose(off);
		for (uint16_t i = 0; i < node->mNumChildren; i++) {
			buildBone child;
			//child.parent = &joint;
			if (readSkeleton(child, node->mChildren[i], offsetMatrices)) {
				joint.childJoints.push_back(child);
			}
		}
		return true;
	}
	else { // find bones in children
		for (uint32_t i = 0; i < node->mNumChildren; i++) {
			if (readSkeleton(joint, node->mChildren[i], offsetMatrices)) {
				return true;
			}

		}
	}
	return false;
}

void FileReader::buildSkeleton(std::vector<Bone>& skeleton, const buildBone& rootBone, int parentID)
{
	Bone newBone;
	newBone.name = rootBone.name;
	newBone.parentIndex = parentID;
	newBone.inverseBindPoseMatrix = rootBone.inverseBindPoseMatrix;
	newBone.id = rootBone.id;
	skeleton[rootBone.id] = newBone;
	for (int i = 0; i < rootBone.childJoints.size(); i++)
	{
		buildSkeleton(skeleton, rootBone.childJoints[i], rootBone.id);
	}
}

void FileReader::loadMesh(
	aiMesh* pMesh, 
	Mesh& mesh, 
	std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>>& boneInfo,
	const bool bones
)
{
	std::vector<SkeletalVertex> skeletalVertices;
	std::vector<Vertex> vertices;

	std::vector<uint32_t> indecies;
	if (bones) {
		skeletalVertices.resize(pMesh->mNumVertices);
	}
	else {
		vertices.resize(pMesh->mNumVertices);
	}
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

		if (pMesh->mTextureCoords[0] != nullptr)
		{
			vert.uv.x = pMesh->mTextureCoords[0][i].x;
			vert.uv.y = pMesh->mTextureCoords[0][i].y;
		}
		else {
			vert.uv.x = 0;
			vert.uv.y = 0;
		}

		if (bones) {
			skeletalVertices[i] = *reinterpret_cast<SkeletalVertex*>(&vert);
			skeletalVertices[i].boneWeight = DirectX::XMFLOAT4(0, 0, 0, 0);
			skeletalVertices[i].boneIds = DirectX::XMINT4(-1, -1, -1, -1);
		}
		else {
			vertices[i] = vert;
		}
	}

	for (uint32_t i = 0; i < pMesh->mNumFaces; i++) {
		indecies.push_back(pMesh->mFaces[i].mIndices[0]);
		indecies.push_back(pMesh->mFaces[i].mIndices[1]);
		indecies.push_back(pMesh->mFaces[i].mIndices[2]);
	}
	if (bones)
	{
		loadBoneDataToVerteies(skeletalVertices, pMesh, boneInfo);
	}

	mesh.nrOfVertecies = pMesh->mNumVertices;
	mesh.nrOfIndecies = (uint32_t)indecies.size();
	if (bones)
	{
		mesh.vertexBuffer = createVertexBuffer(
			skeletalVertices,
			gfx->getDevice(),
			gfx->getCommandList(),
			gfx->getCommandAllocator(),
			gfx->getCommandQueue(),
			gfx->getFenceEvent(),
			gfx->getFence(),
			gfx->getFenceValue()
		);
	}
	else {
		mesh.vertexBuffer = createVertexBuffer(
			vertices,
			gfx->getDevice(),
			gfx->getCommandList(),
			gfx->getCommandAllocator(),
			gfx->getCommandQueue(),
			gfx->getFenceEvent(),
			gfx->getFence(),
			gfx->getFenceValue()
		);
	}
	mesh.indeciesBuffer = createIndeciesBuffer(
		indecies,
		gfx->getDevice(),
		gfx->getCommandList(),//unsure If I should I have a number here?
		gfx->getCommandAllocator(),//unsure If I should I have a number here?
		gfx->getCommandQueue(),
		gfx->getFenceEvent(),
		gfx->getFence(),
		gfx->getFenceValue()
	);

	mesh.vertexBufferView.BufferLocation = mesh.vertexBuffer->GetGPUVirtualAddress();
	if (bones)
	{
		mesh.vertexBufferView.SizeInBytes = (uint32_t)skeletalVertices.size() * sizeof(SkeletalVertex);
		mesh.vertexBufferView.StrideInBytes = sizeof(SkeletalVertex);
	}
	else {
		mesh.vertexBufferView.SizeInBytes = (uint32_t)vertices.size() * sizeof(Vertex);
		mesh.vertexBufferView.StrideInBytes = sizeof(Vertex);
	}
	

	mesh.indeciesBufferView.BufferLocation = mesh.indeciesBuffer->GetGPUVirtualAddress();
	mesh.indeciesBufferView.SizeInBytes = (UINT)(indecies.size() * sizeof(uint32_t));
	mesh.indeciesBufferView.Format = DXGI_FORMAT_R32_UINT;

}

#include <fstream>
void writeVerteciesToFile(std::vector<SkeletalVertex>& skeletalVerticies)
{
	std::ofstream file("vertecies.txt");
	if (!file.is_open()) {
		std::cerr << "Failed to open skeletonTest.txt" << std::endl;
		return;
	}
	for (int i = 0; i < skeletalVerticies.size(); i++)
	{
		file << skeletalVerticies[i].boneIds.x << skeletalVerticies[i].boneIds.y << skeletalVerticies[i].boneIds.z << skeletalVerticies[i].boneIds.w << 
			"," << skeletalVerticies[i].boneWeight.x << skeletalVerticies[i].boneWeight.y << skeletalVerticies[i].boneWeight.z << skeletalVerticies[i].boneWeight.w << "\n";
	}
	file.close();
}

void FileReader::loadBoneDataToVerteies(
	std::vector<SkeletalVertex>& skeletalVerticies, 
	const aiMesh* pMesh, 
	std::unordered_map<std::string, std::pair<int, DirectX::XMMATRIX>>& offsetMatrices)
{
	const uint8_t HowManyBonesNow = offsetMatrices.size();
	std::vector<uint16_t> boneCounts;
	boneCounts.resize(skeletalVerticies.size());

	for (uint8_t i = 0; i < pMesh->mNumBones; i++) {
		aiBone* bone = pMesh->mBones[i];
		//int boneID;
		//if (offsetMatrices.find(bone->mName.C_Str()) == offsetMatrices.end())
		//{
			offsetMatrices[bone->mName.C_Str()] = { i, ConvertToXMMATRIX(bone->mOffsetMatrix) };
		//}
		//boneID = offsetMatrices[bone->mName.C_Str()].first;

		for (uint32_t w = 0; w < bone->mNumWeights; w++) {
			uint32_t id = bone->mWeights[w].mVertexId;
			float weight = bone->mWeights[w].mWeight;

			boneCounts[id]++;
			switch (boneCounts[id]) {
			case 1:
				skeletalVerticies[id].boneIds.x = i;
				skeletalVerticies[id].boneWeight.x = weight;
				break;
			case 2:
				skeletalVerticies[id].boneIds.y = i;
				skeletalVerticies[id].boneWeight.y = weight;
				break;
			case 3:
				skeletalVerticies[id].boneIds.z = i;
				skeletalVerticies[id].boneWeight.z = weight;
				break;
			case 4:
				skeletalVerticies[id].boneIds.w = i;
				skeletalVerticies[id].boneWeight.w = weight;
				break;
			default:
				break;

			}
		}
	}
	//normalize weight in vertecies
	for (int i = 0; i < skeletalVerticies.size(); i++) {
		float totalWeight =
			skeletalVerticies[i].boneWeight.x +
			skeletalVerticies[i].boneWeight.y +
			skeletalVerticies[i].boneWeight.z +
			skeletalVerticies[i].boneWeight.w;

		skeletalVerticies[i].boneWeight.x = skeletalVerticies[i].boneWeight.x / totalWeight;
		skeletalVerticies[i].boneWeight.y = skeletalVerticies[i].boneWeight.y / totalWeight;
		skeletalVerticies[i].boneWeight.z = skeletalVerticies[i].boneWeight.z / totalWeight;
		skeletalVerticies[i].boneWeight.w = skeletalVerticies[i].boneWeight.w / totalWeight;
	}
}

