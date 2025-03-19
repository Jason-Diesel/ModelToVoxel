#include "VoxelShaderClass.h"

void VoxelShaderClass::init(
	ID3D12Device8* device, 
	const uint32_t nrOfConstantBufferViews,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& customInputLayout,
	const std::string& vertexShader, 
	const std::string& pixelShader, 
	bool wireFrame
)
{
	shaderTypes st = (shaderTypes)((int)shaderTypes::_VertexShader + (int)shaderTypes::_PixelShader);
	createRootSignature(device, nrOfConstantBufferViews, st, 1);
	const std::vector<std::string> files = { vertexShader, pixelShader };
	createPipelineState(device, customInputLayout, files, st, wireFrame);
}

void VoxelShaderClass::createRootSignature(ID3D12Device8* device, const uint32_t nrOfConstantBufferViews, const shaderTypes shaderTypeFlag, const uint32_t nrOfMaterials)
{
	int index = 0;
	std::vector<CD3DX12_ROOT_PARAMETER> rootParameters(nrOfConstantBufferViews + EXTRACONSTANTBUFFERVIEWS + 1 + nrOfMaterials);
	std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges(nrOfMaterials);
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

	//HERE IS TRANSFORM
	rootParameters[descriptorIndex++].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 5, 0);

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

