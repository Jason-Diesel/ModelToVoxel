#include "Shader.h"
#include "HelperFuncitons.h"
#include "Defines.h"

//TODO : NEED TO MAKE A LINK TO WHAT THINGS

Shader::Shader()
{
}

Shader::Shader(
	ID3D12Device8* device, 
	const uint32_t nrOfConstantBufferViews, 
	const std::vector<MaterialDescription>& MaterialDescriptions,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, 
	const std::string& vertexShader, 
	const std::string& pixelShader, 
	bool wireFrame
)
{
	init(
		device,
		nrOfConstantBufferViews,
		MaterialDescriptions,
		customInputLayout,
		vertexShader,
		pixelShader,
		wireFrame
	);
}

void Shader::init(
	ID3D12Device8* device,
	const uint32_t nrOfConstantBufferViews,
	const std::vector<MaterialDescription>& MaterialDescriptions,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
	const std::string& vertexShader,
	const std::string& pixelShader,
	bool wireFrame)
{
	shaderTypes st = (shaderTypes)((int)shaderTypes::_VertexShader | (int)shaderTypes::_PixelShader);
	createRootSignature(device, nrOfConstantBufferViews, st, MaterialDescriptions);
	const std::vector<std::string> files = { vertexShader, pixelShader };
	
	createPipelineState(device, customInputLayout, files, st, wireFrame);
}

void Shader::init(
	ID3D12Device8* device, 
	const uint32_t nrOfConstantBufferViews,
	const std::vector<MaterialDescription>& MaterialDescriptions, 
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, 
	const std::string& vertexShader, 
	const std::string& hullShader, 
	const std::string& domainShader, 
	const std::string& pixelShader, 
	bool wireFrame
)
{
	shaderTypes st = (shaderTypes)((int)shaderTypes::_VertexShader + (int)shaderTypes::_DomainShader + (int)shaderTypes::_HullShader + (int)shaderTypes::_PixelShader);
	createRootSignature(device, nrOfConstantBufferViews, st, MaterialDescriptions);
	const std::vector<std::string> files = {vertexShader, hullShader, domainShader, pixelShader};
	createPipelineState(device, customInputLayout, files, st, wireFrame);
}

void Shader::init(ID3D12Device8* device, const uint32_t nrOfConstantBufferViews, const std::vector<MaterialDescription>& MaterialDescriptions, const std::string& computeShader)
{
	shaderTypes st = shaderTypes::_ComputeShader;
	createRootSignatureComputeShader(device, nrOfConstantBufferViews, MaterialDescriptions);
	createPipelineStateComputeShader(device, computeShader);
}

void Shader::initShadow(
	ID3D12Device8* device, 
	const uint32_t nrOfConstantBufferViews, 
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, 
	const std::string& vertexShader, 
	const std::string& pixelShader)
{
	createRootSignatureShadow(device, nrOfConstantBufferViews);
	createPipelineState(device, customInputLayout, vertexShader, pixelShader, false);
}

ID3D12RootSignature* Shader::getRootSignature()
{
	return rootSignature.Get();
}

ID3D12PipelineState* Shader::getPipeLineState()
{
	return pipelineState.Get();
}

void Shader::createRootSignature(
	ID3D12Device8* device, 
	const uint32_t nrOfConstantBufferViews,
	const shaderTypes shaderTypeFlag,
	std::vector<MaterialDescription> materialDescription
)
{
	int index = 0;
	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters(nrOfConstantBufferViews + EXTRACONSTANTBUFFERVIEWS + materialDescription.size());
	std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges(materialDescription.size());
	int descriptorIndex = 0;
	//for all def constantbuffers
	for (uint32_t i = 0; i < EXTRACONSTANTBUFFERVIEWS; i++)
	{
		rootParameters[descriptorIndex++].InitAsConstantBufferView(
			i,
			0
		);
	}

	//BindLess TextureHeap
	descriptorRanges[index].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		MAXNROFMATERIALS,
		0
	);
	rootParameters[descriptorIndex++].InitAsDescriptorTable(1, &descriptorRanges[index++], D3D12_SHADER_VISIBILITY_ALL);

	//all other constantBuffers
	for (uint32_t i = 0; i < nrOfConstantBufferViews; i++)
	{
		rootParameters[descriptorIndex++].InitAsConstantBufferView(
			i + EXTRACONSTANTBUFFERVIEWS,
			0
		);
	}

	CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
	
	D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
	if (!(shaderTypeFlag & _HullShader))
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	}
	if (!(shaderTypeFlag & _DomainShader))
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	}
	if (!(shaderTypeFlag & _GeometryShader))
	{
		flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	}

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
		static_cast<UINT>(rootParameters.size()), rootParameters.data(),
		1, &staticSampler,
		flags
	);
	
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob, &errorBlob
	);
	
	if (FAILED(hr)) {
		if (errorBlob) {
			const char* errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());
			std::cerr << "Error: " << errorBufferPtr << std::endl;
		}
		CheckHR(hr);
	}
	
	CheckHR(device->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)
	));
}

void Shader::createRootSignatureShadow(ID3D12Device8* device, const uint32_t nrOfConstantBufferViews)
{
	int index = 0;
	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters(nrOfConstantBufferViews + EXTRACONSTANTBUFFERVIEWS);
	int descriptorIndex = 0;
	//for all constantbuffers
	for (uint32_t i = 0; i < nrOfConstantBufferViews + EXTRACONSTANTBUFFERVIEWS; i++)
	{
		rootParameters[descriptorIndex++].InitAsConstantBufferView(
			i,
			0
		);
	}

	CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
		static_cast<UINT>(rootParameters.size()), rootParameters.data(),
		1, &staticSampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
	);

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob, &errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			const char* errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());
			std::cerr << "Error: " << errorBufferPtr << std::endl;
		}
		CheckHR(hr);
	}

	CheckHR(device->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)
	));
}

void Shader::createRootSignatureComputeShader(ID3D12Device8* device, const uint32_t nrOfConstantBufferViews, std::vector<MaterialDescription> materialDescription)
{
	int index = 0;
	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters(nrOfConstantBufferViews + materialDescription.size());
	std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges(materialDescription.size());
	int descriptorIndex = 0;

	for (uint32_t i = 0; i < nrOfConstantBufferViews; i++)
	{
		rootParameters[descriptorIndex++].InitAsConstantBufferView(
			i,
			0
		);
	}

	//BindLess TextureHeap
	for (int i = 0; i < materialDescription.size(); i++)
	{
		descriptorRanges[index].Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			materialDescription[i].NrOfTextures,
			i
		);
		rootParameters[descriptorIndex++].InitAsDescriptorTable(1, &descriptorRanges[index++], D3D12_SHADER_VISIBILITY_ALL);
	}


	CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };

	const D3D12_ROOT_SIGNATURE_FLAGS flags = 
		D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
		static_cast<UINT>(rootParameters.size()), rootParameters.data(),
		1, &staticSampler,
		flags
	);

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob, &errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			const char* errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());
			std::cerr << "Error: " << errorBufferPtr << std::endl;
		}
		CheckHR(hr);
	}

	CheckHR(device->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)
	));
}

//SHADOW
void Shader::createPipelineState(
	ID3D12Device8* device, 
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, 
	const std::string& vertexShaderStr, 
	const std::string& pixelShaderStr, 
	const bool& wireFrame
)
{
	struct PipelineStateStream {
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT inputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY topology;
		CD3DX12_PIPELINE_STATE_STREAM_VS vs;
		CD3DX12_PIPELINE_STATE_STREAM_PS ps;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtvFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC blend;
	} pipelineStateStream;

	std::string filePath;
#ifdef _DEBUG
	filePath = "../x64/Debug/";
	//filePath = "";
#else
	filePath = "../x64/Release/";
	//filePath = "";
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

	D3DReadFileToBlob(createStringToWString(filePath + vertexShaderStr).c_str(), &vertexShader);
	D3DReadFileToBlob(createStringToWString(filePath + pixelShaderStr).c_str(), &pixelShader);

	pipelineStateStream.rootSignature = rootSignature.Get();
	pipelineStateStream.inputLayout = { customInputLayout.data(), static_cast<UINT>(customInputLayout.size()) };
	pipelineStateStream.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.vs = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	pipelineStateStream.ps = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.rtvFormat = {
		.RTFormats { DXGI_FORMAT_R8G8B8A8_UNORM },
		.NumRenderTargets = 1
	};
	this->topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	if (wireFrame)
	{
		rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	}

	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	pipelineStateStream.rasterizer = rasterizerDesc;

	CD3DX12_BLEND_DESC blendDescription(D3D12_DEFAULT);
	blendDescription.AlphaToCoverageEnable = FALSE;
	blendDescription.IndependentBlendEnable = FALSE;

	const D3D12_RENDER_TARGET_BLEND_DESC defaultBlendDesc = {
	TRUE,                          // Enable blending
	FALSE,
	D3D12_BLEND_SRC_ALPHA,         // Source blend factor
	D3D12_BLEND_INV_SRC_ALPHA,     // Destination blend factor
	D3D12_BLEND_OP_ADD,            // Blend operation
	D3D12_BLEND_ONE,               // Source alpha blend factor
	D3D12_BLEND_ZERO,              // Destination alpha blend factor
	D3D12_BLEND_OP_ADD,            // Alpha blend operation
	D3D12_LOGIC_OP_NOOP,
	D3D12_COLOR_WRITE_ENABLE_ALL,  // Write mask
	};
	blendDescription.RenderTarget[0] = defaultBlendDesc;
	pipelineStateStream.blend = blendDescription;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	CheckHR(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState)));
}

void Shader::createPipelineState(
	ID3D12Device8* device, 
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, 
	const std::vector<std::string>& shaderFiles, 
	const shaderTypes shaderTypeFlag, 
	const bool& wireFrame
)
{
	struct PipelineStateStream {
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT inputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY topology;
		CD3DX12_PIPELINE_STATE_STREAM_VS vs = CD3DX12_SHADER_BYTECODE(nullptr, 0);
		CD3DX12_PIPELINE_STATE_STREAM_HS hs = CD3DX12_SHADER_BYTECODE(nullptr, 0);
		CD3DX12_PIPELINE_STATE_STREAM_DS ds = CD3DX12_SHADER_BYTECODE(nullptr, 0);
		CD3DX12_PIPELINE_STATE_STREAM_PS ps = CD3DX12_SHADER_BYTECODE(nullptr, 0);
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtvFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER rasterizer;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC blend;
	} pipelineStateStream;

	std::string filePath;
#ifdef _DEBUG
	filePath = "../x64/Debug/";
	//filePath = "";
#else
	filePath = "../x64/Release/";
	//filePath = "";
#endif
	std::vector<Microsoft::WRL::ComPtr<ID3DBlob>> shaderBlobs;
	shaderBlobs.resize(shaderFiles.size());

	for (int i = 0; i < shaderFiles.size(); i++)
	{
		D3DReadFileToBlob(createStringToWString(filePath + shaderFiles[i]).c_str(), &shaderBlobs[i]);
	}

	pipelineStateStream.rootSignature = rootSignature.Get();
	pipelineStateStream.inputLayout = { customInputLayout.data(), static_cast<UINT>(customInputLayout.size()) };
	pipelineStateStream.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	this->topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	int i = 0;
	if (shaderTypeFlag & _VertexShader)
	{
		pipelineStateStream.vs = CD3DX12_SHADER_BYTECODE(shaderBlobs[i++].Get());
	}
	if (shaderTypeFlag & _HullShader)
	{
		pipelineStateStream.hs = CD3DX12_SHADER_BYTECODE(shaderBlobs[i++].Get());
		pipelineStateStream.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		topology = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	}
	if (shaderTypeFlag & _DomainShader)
	{
		pipelineStateStream.ds = CD3DX12_SHADER_BYTECODE(shaderBlobs[i++].Get());
	}
	if (shaderTypeFlag & _PixelShader)
	{
		pipelineStateStream.ps = CD3DX12_SHADER_BYTECODE(shaderBlobs[i++].Get());
	}
	
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.rtvFormat = {
		.RTFormats { DXGI_FORMAT_R8G8B8A8_UNORM },
		.NumRenderTargets = 1
	};

	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	if (wireFrame)
	{
		rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	}
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	pipelineStateStream.rasterizer = rasterizerDesc;

	CD3DX12_BLEND_DESC blendDescription(D3D12_DEFAULT);
	blendDescription.AlphaToCoverageEnable = FALSE;
	blendDescription.IndependentBlendEnable = FALSE;

	const D3D12_RENDER_TARGET_BLEND_DESC defaultBlendDesc = {
	TRUE,                          // Enable blending
	FALSE,
	D3D12_BLEND_SRC_ALPHA,         // Source blend factor
	D3D12_BLEND_INV_SRC_ALPHA,     // Destination blend factor
	D3D12_BLEND_OP_ADD,            // Blend operation
	D3D12_BLEND_ONE,               // Source alpha blend factor
	D3D12_BLEND_ZERO,              // Destination alpha blend factor
	D3D12_BLEND_OP_ADD,            // Alpha blend operation
	D3D12_LOGIC_OP_NOOP,
	D3D12_COLOR_WRITE_ENABLE_ALL,  // Write mask
	};
	blendDescription.RenderTarget[0] = defaultBlendDesc;
	pipelineStateStream.blend = blendDescription;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	CheckHR(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState)));
}

void Shader::createPipelineStateComputeShader(ID3D12Device8* device, const std::string& shaderFile)
{
	std::string filePath;
#ifdef _DEBUG
	filePath = "../x64/Debug/";
	//filePath = "";
#else
	filePath = "../x64/Release/";
	//filePath = "";
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;

	D3DReadFileToBlob(createStringToWString(filePath + shaderFile).c_str(), &shaderBlob);

	D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateStreamDesc = {};
	pipelineStateStreamDesc.pRootSignature = rootSignature.Get();
	pipelineStateStreamDesc.CS = CD3DX12_SHADER_BYTECODE(shaderBlob.Get());
	pipelineStateStreamDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	pipelineStateStreamDesc.NodeMask = 0;

	device->CreateComputePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState));
}


D3D12_PRIMITIVE_TOPOLOGY Shader::getTopology()
{
	return this->topology;
}

void Shader::setTopology(const D3D12_PRIMITIVE_TOPOLOGY topology)
{
	this->topology = topology;
}
