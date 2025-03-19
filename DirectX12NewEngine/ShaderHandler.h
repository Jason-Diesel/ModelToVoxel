#pragma once
#include "Graphics.h"
#include "Shader.h"
#include "Renderer.h"
#include "ConstantBuffers.h"
#include "LightHandler.h"


class ShaderHandler {
public:
	ShaderHandler(
		Graphics* gfx, 
		Renderer* renderer,
		LightHandler* lightHandler
	);
	void init();
	int createShader(
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& Materials,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader,
		bool wireFrame = false
	);
	int createShader(
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& Materials,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& hullShader,
		const std::string& domainShader,
		const std::string& pixelShader,
		bool wireFrame = false
	);
	int createNoLightShader(
		const uint32_t nrOfConstantBufferViews,
		const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
		const std::string& vertexShader,
		const std::string& pixelShader
	);
	int createShader(//COMPUTE SHADER
		const uint32_t nrOfConstantBufferViews,
		const std::vector<MaterialDescription>& Materials,
		const std::string& computeShader
	);
	Shader& getShader(uint32_t shaderIndex);
	void setShadowShaders(const uint32_t shaderIndex);
	void setShader(const uint32_t shaderIndex);
	void setShader(Shader& shader);
	void setComputeShader(const uint32_t shaderIndex);
private:
	Graphics* gfx;
	Renderer* renderer;
	LightHandler* lightHandler;
	std::vector<Shader> shaders;
};
