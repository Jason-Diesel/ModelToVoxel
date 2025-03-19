#pragma once
#include "Shader.h"

//SPECIAL SHADER FOR VOXELS
class VoxelShaderClass : public Shader {
public:
	void init(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader,
		bool wireFrame = false
	);
private:
	void createRootSignature(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const shaderTypes shaderTypeFlag,
		const uint32_t nrOfMaterials
	);
};