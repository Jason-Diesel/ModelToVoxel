#include "SpecialVoxelShader.h"

void SpecialVoxelShader::createRootSignatureS(
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
	descriptorRanges[index].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		1,
		0,
		0
	);
	rootParameters[descriptorIndex++].InitAsDescriptorTable(1, &descriptorRanges[index++], D3D12_SHADER_VISIBILITY_ALL);

	//BindLess TextureHeap
	descriptorRanges[index].Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		MAXNROFMATERIALS,
		1,
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

void SpecialVoxelShader::createRootSignatureShadow(
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

	descriptorRanges[index].Init(
		//D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		1,
		0,
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

void SpecialVoxelShader::init(
	ID3D12Device8* device, 
	const uint32_t nrOfConstantBufferViews, 
	const std::vector<MaterialDescription>& MaterialDescriptions, 
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, 
	const std::string& vertexShader, 
	const std::string& pixelShader, 
	bool wireFrame
)
{
	shaderTypes st = (shaderTypes)((int)shaderTypes::_VertexShader + (int)shaderTypes::_PixelShader);
	this->createRootSignatureS(device, nrOfConstantBufferViews, st, MaterialDescriptions);
	const std::vector<std::string> files = { vertexShader, pixelShader };
	createPipelineState(device, customInputLayout, files, st, wireFrame);
}

void SpecialVoxelShader::initS(
	ID3D12Device8* device, 
	const uint32_t nrOfConstantBufferViews, 
	const std::vector<MaterialDescription>& MaterialDescriptions, 
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout, 
	const std::string& vertexShader, 
	const std::string& pixelShader, 
	bool wireFrame)
{
	shaderTypes st = (shaderTypes)((int)shaderTypes::_VertexShader + (int)shaderTypes::_PixelShader);
	createRootSignatureShadow(device, nrOfConstantBufferViews, st, MaterialDescriptions);
	const std::vector<std::string> files = { vertexShader, pixelShader };
	createPipelineState(device, customInputLayout, files, st, wireFrame);
}
