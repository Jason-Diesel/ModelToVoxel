#include "ShaderHandler.h"

ShaderHandler::ShaderHandler(
	Graphics* gfx, 
	Renderer* renderer, 
	LightHandler* lightHandler
): gfx(gfx), renderer(renderer), lightHandler(lightHandler)
{
}

void ShaderHandler::init()
{
	//create a default shader
	std::vector<MaterialDescription> aMaterial;
	aMaterial.push_back(MaterialDescription(1));
	this->createNoLightShader(0, gfx->getInputLayout(0), "VertexShadow.cso", "PixelShadow.cso");
	this->createShader(0, aMaterial, gfx->getInputLayout(0), "VertexShader.cso", "PixelShader.cso");
}

int ShaderHandler::createShader(
	const uint32_t nrOfConstantBufferViews, 
	const std::vector<MaterialDescription>& Materials, 
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, 
	const std::string& vertexShader, 
	const std::string& pixelShader, 
	bool wireFrame
)
{
	{
		const int shader = gotShader(vertexShader + pixelShader);
		if (shader >= 0)
		{
			return shader;
		}
	}
	shaders.resize(shaders.size() + 1);
	shaders.back().init(
		gfx->getDevice(),
		nrOfConstantBufferViews,
		Materials,
		customInputLayout,
		vertexShader,
		pixelShader,
		wireFrame
		);

	GottenShaders.insert(std::pair<std::string, int>(vertexShader + pixelShader, shaders.size() - 1));
	return static_cast<uint32_t>(shaders.size() - 1);
}

int ShaderHandler::createShader(
	const uint32_t nrOfConstantBufferViews,
	const std::vector<MaterialDescription>& Materials,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
	const std::string& vertexShader,
	const std::string& hullShader,
	const std::string& domainShader,
	const std::string& pixelShader,
	bool wireFrame
)
{
	{
		const int shader = gotShader(vertexShader + hullShader + domainShader + pixelShader);
		if (shader >= 0)
		{
			return shader;
		}
	}

	shaders.resize(shaders.size() + 1);
	shaders.back().init(
		gfx->getDevice(),
		nrOfConstantBufferViews,
		Materials,
		customInputLayout,
		vertexShader,
		hullShader,
		domainShader,
		pixelShader,
		wireFrame
	);
	GottenShaders.insert(std::pair<std::string, int>(vertexShader + hullShader + domainShader + pixelShader, shaders.size() - 1));
	return static_cast<uint32_t>(shaders.size() - 1);
}

int ShaderHandler::createNoLightShader(const uint32_t nrOfConstantBufferViews, const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, const std::string& vertexShader, const std::string& pixelShader)
{
	{
		const int shader = gotShader(vertexShader + pixelShader);
		if (shader >= 0)
		{
			return shader;
		}
	}

	shaders.resize(shaders.size() + 1);
	shaders.back().initShadow(
		gfx->getDevice(),
		nrOfConstantBufferViews,
		customInputLayout,
		vertexShader,
		pixelShader
	);

	GottenShaders.insert(std::pair<std::string, int>(vertexShader + pixelShader, shaders.size() - 1));
	return static_cast<uint32_t>(shaders.size() - 1);
}

int ShaderHandler::createShader(const uint32_t nrOfConstantBufferViews, const std::vector<MaterialDescription>& Materials, const std::string& computeShader)
{
	{
		const int shader = gotShader(computeShader);
		if (shader >= 0)
		{
			return shader;
		}
	}

	shaders.resize(shaders.size() + 1);
	shaders.back().init(
		gfx->getDevice(),
		nrOfConstantBufferViews,
		Materials,
		computeShader
	);
	GottenShaders.insert(std::pair<std::string, int>(computeShader, shaders.size() - 1));

	return static_cast<uint32_t>(shaders.size() - 1);
}

Shader& ShaderHandler::getShader(uint32_t shaderIndex)
{
	return shaders[shaderIndex];
}

void ShaderHandler::setShadowShaders(const uint32_t shaderIndex)
{
	if (!renderer->isMakingShadows())
	{
		return;
	}
	gfx->getCommandList()->IASetPrimitiveTopology(shaders[shaderIndex].getTopology());
	gfx->getCommandList()->SetPipelineState(shaders[shaderIndex].getPipeLineState());
	gfx->getCommandList()->SetGraphicsRootSignature(shaders[shaderIndex].getRootSignature());
}

void ShaderHandler::setShadowShaders(Shader& shader)
{
	if (!renderer->isMakingShadows())
	{
		return;
	}
	gfx->getCommandList()->IASetPrimitiveTopology(shader.getTopology());
	gfx->getCommandList()->SetPipelineState(shader.getPipeLineState());
	gfx->getCommandList()->SetGraphicsRootSignature(shader.getRootSignature());
}

void ShaderHandler::setShader(const uint32_t shaderIndex)
{
	if (renderer->isMakingShadows())
	{
		return;
	}

	gfx->getCommandList()->IASetPrimitiveTopology(shaders[shaderIndex].getTopology());
	gfx->getCommandList()->SetPipelineState(shaders[shaderIndex].getPipeLineState());
	gfx->getCommandList()->SetGraphicsRootSignature(shaders[shaderIndex].getRootSignature());
	
	//TODO : I DON'T KNOW EXACTLY WHAT SHOULD BE HERE
	this->renderer->updateShader();
	lightHandler->setAsGraphicRoot();
	gfx->setCurrentTextureHeap();
	/////////////////////////////////////////////////
}

void ShaderHandler::setShader(Shader& shader)
{
	gfx->getCommandList()->IASetPrimitiveTopology(shader.getTopology());
	gfx->getCommandList()->SetPipelineState(shader.getPipeLineState());
	gfx->getCommandList()->SetGraphicsRootSignature(shader.getRootSignature());

	//TODO : I DON'T KNOW EXACTLY WHAT SHOULD BE HERE
	this->renderer->updateShader();
	lightHandler->setAsGraphicRoot();
	gfx->setCurrentTextureHeap();
	/////////////////////////////////////////////////
}

void ShaderHandler::setComputeShader(const uint32_t shaderIndex)
{
	gfx->getCommandList()->SetPipelineState(shaders[shaderIndex].getPipeLineState());
	gfx->getCommandList()->SetComputeRootSignature(shaders[shaderIndex].getRootSignature());
}

int ShaderHandler::gotShader(const std::string shader)
{
	if (GottenShaders.contains(shader))
	{
		return GottenShaders.find(shader)->second;
	}
	//if we don't have it return -1
	return -1;
}
