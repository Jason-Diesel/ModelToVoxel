#pragma once
#include "Shader.h"

class SpecialVoxelShader : public Shader 
{
public:
	void createRootSignatureS(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const shaderTypes shaderTypeFlag,
		std::vector<MaterialDescription> materialDescription
	);
	void initS(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& MaterialDescriptions,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader,
		bool wireFrame = false
	);
};