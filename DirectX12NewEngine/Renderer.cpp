#include "Renderer.h"

Renderer::Renderer(Graphics* gfx) :
	gfx(gfx),
	shadowMode(false)
{
	
}

Renderer::~Renderer()
{
	viewProjectionBuffer.constantBuffer->Release();
	transformBuffer.constantBuffer->Release();
}

void Renderer::init()
{
	viewProjectionBuffer = CreateConstantBuffer<ViewProjConstantBufferData>(gfx);
	transformBuffer = CreateConstantBuffer<DirectX::XMMATRIX>(gfx, DirectX::XMMatrixIdentity());
}

void Renderer::ShadowReady()
{
	shadowMode = true;
}

void Renderer::setReady()
{
	shadowMode = false;
	if (currentCamera == nullptr) { return; }

	currentCamera->update();
	updateConstantBuffer(currentCamera->getViewProjConstantBufferData(), viewProjectionBuffer);
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(0, viewProjectionBuffer.constantBuffer->GetGPUVirtualAddress());
}

void Renderer::updateShader()
{
	if (currentCamera == nullptr) { return; }
	gfx->getCommandList()->SetGraphicsRootConstantBufferView(0, viewProjectionBuffer.constantBuffer->GetGPUVirtualAddress());
}

void Renderer::render(Object* object, Model* customModel)
{
	if (customModel == nullptr) { return; }

	object->setConstantBuffer(gfx);
	if (customModel->ModelType == Model::_AnimatedModel)
	{
		AnimationComponent* animatedComponent = object->getComponent<AnimationComponent>();
		animatedComponent->setPose((AnimatedModel*)customModel);
		gfx->getCommandList()->SetGraphicsRootConstantBufferView(5, animatedComponent->skeletalConstantBuffer.constantBuffer->GetGPUVirtualAddress());
	}

	render(customModel);
}

void Renderer::render(Object* object)
{
	Model* model = object->getComponent<Model>();
	if (model == nullptr) { return; }

	object->setConstantBuffer(gfx);
	if (model->ModelType == Model::_AnimatedModel)
	{
		AnimationComponent* animatedComponent = object->getComponent<AnimationComponent>();
		animatedComponent->setPose((AnimatedModel*)model);
		gfx->getCommandList()->SetGraphicsRootConstantBufferView(5, animatedComponent->skeletalConstantBuffer.constantBuffer->GetGPUVirtualAddress());
	}

	render(model);
}

void Renderer::render(Model* model)
{
	for (uint32_t i = 0; i < model->nrOfSubMeshes; i++)
	{
		render(model->subMeshes[i]);
	}
}

void Renderer::render(Mesh& mesh)
{
	if (!shadowMode && mesh.material != nullptr && mesh.material->getNrOfTextures() > 0)
	{
		mesh.material->setMaterial(gfx);
	}

	gfx->getCommandList()->IASetVertexBuffers(0, 1, &mesh.vertexBufferView);
	gfx->getCommandList()->IASetIndexBuffer(&mesh.indeciesBufferView);
	gfx->getCommandList()->DrawIndexedInstanced(mesh.nrOfIndecies, 1, 0, 0, 0);
}

void Renderer::setCurrentCamera(Camera* camera)
{
	currentCamera = camera;
}

const bool Renderer::isMakingShadows()
{
	return shadowMode;
}
