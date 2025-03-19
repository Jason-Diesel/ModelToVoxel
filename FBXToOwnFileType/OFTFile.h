#pragma once

enum MaterialFlags {
	None = 0,
	DiffuseTexture	= 1 << 0,
	NormalTexture	= 1 << 1,
	HeightTexture	= 1 << 2,
	LightTexture	= 1 << 3,
};

static const MaterialFlags MaterialFlagsArray[] = { DiffuseTexture, NormalTexture, HeightTexture, LightTexture };

//How many meshes			(uint32_t)
//For how many meshes		
//	How many of vertecies		(uint32_t)
//	VertexData
//	Size of indeciesData
//	indeciesData
//	MaterialIndex < 0 means it doesn't have a material
// 
// Small test here should be 696969
// 
//NrOfTextures
// Textures paths
//How many materials
//For how many materials
// Ka,Kd,Ks,Ns
// NrOfTextures?? (Do I need that, faster loading time so yes?) (-1 doesn't have a texture)
// TexturesTypes
// In order texture index
//