#pragma once
#include "Shader.h"

class SpecialVoxelShader : public Shader 
{
public:
	
	void init(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& MaterialDescriptions,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader,
		bool wireFrame = false
	);
	void initS(//FOR SHADOWS
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& MaterialDescriptions,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader,
		bool wireFrame = false
	);
private:
	void createRootSignatureS(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const shaderTypes shaderTypeFlag,
		std::vector<MaterialDescription> materialDescription
	);
	void createRootSignatureShadow(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const shaderTypes shaderTypeFlag,
		std::vector<MaterialDescription> materialDescription
	);
};