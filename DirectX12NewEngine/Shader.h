#pragma once
#include "CommonHeaders.h"
#include "Material.h"
#include <unordered_map>

class Shader
{
public:
	Shader();
	Shader(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& MaterialDescriptions,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader,
		bool wireFrame = false);
	void init(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& MaterialDescriptions,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader,
		bool wireFrame = false
	);
	void init(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& MaterialDescriptions,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& hullShader,
		const std::string& domainShader,
		const std::string& pixelShader,
		bool wireFrame = false
	);
	void init(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& MaterialDescriptions,
		const std::string& computeShader
	);
	void initShadow(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader
	);
	ID3D12RootSignature* getRootSignature();
	ID3D12PipelineState* getPipeLineState();
protected:
	enum shaderTypes {
		None = 0,
		_VertexShader = 1 << 0,
		_HullShader = 1 << 1,
		_DomainShader = 1 << 2,
		_GeometryShader = 1 << 3,
		_PixelShader = 1 << 4,
		_ComputeShader = 1 << 5,
	};
protected:
	void createRootSignature(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		const shaderTypes shaderTypeFlag,
		std::vector<MaterialDescription> materialDescription
	);
	void createPipelineState(
		ID3D12Device8* device,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::vector<std::string>& shaderFiles,
		const shaderTypes shaderTypeFlag,
		const bool& wireFrame = false
	);
private:
	void createRootSignatureShadow(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews
	);
	//TODO : REMOVE THIS, ONLY SHADOW IS USING IT, BUT SHOULDN'T EVEN BE USED BY SHADOW
	void createPipelineState(
		ID3D12Device8* device,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShaderStr,
		const std::string& pixelShaderStr,
		const bool& wireFrame = false
	);
private:
	void createRootSignatureComputeShader(
		ID3D12Device8* device,
		const uint32_t nrOfConstantBufferViews,
		std::vector<MaterialDescription> materialDescription
	);

	void createPipelineStateComputeShader(
		ID3D12Device8* device,
		const std::string& shaderFile
	);
public:
	D3D12_PRIMITIVE_TOPOLOGY getTopology();
	void setTopology(const D3D12_PRIMITIVE_TOPOLOGY topology);
protected:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	D3D12_PRIMITIVE_TOPOLOGY topology;
};