#include "IMGUIHandler.h"
#include "SceneManager.h"
IMGUIHandler::IMGUIHandler(Graphics* gfx, Window* window, SceneManager* sceneManager)
{
	this->gfx = gfx;
	this->window = window;
	this->sceneManager = sceneManager;
}

IMGUIHandler::~IMGUIHandler()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void IMGUIHandler::init()
{
	// Initialize Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window->getRenderWindow().getHandle());
	ImGui_ImplDX12_Init(
		gfx->getDevice(),                     // ID3D12Device pointer
		2,                                   // Number of frames in the swap chain
		DXGI_FORMAT_R8G8B8A8_UNORM,          // Render target format
		gfx->IMGUIgetDescriptorHeap(),        // Descriptor heap for ImGui
		gfx->IMGUIgetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		gfx->IMGUIgetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart()
	);

	ShowWindow(window->getRenderWindow().getHandle(), SW_SHOWDEFAULT);
	UpdateWindow(window->getRenderWindow().getHandle());
}

void IMGUIHandler::startRender()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void IMGUIHandler::endRender()
{
	// Render ImGui
	ImGui::Render();
	ID3D12DescriptorHeap* heap = gfx->IMGUIgetDescriptorHeap();
	gfx->getCommandList()->SetDescriptorHeaps(1, &heap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gfx->getCommandList());
}

void IMGUIHandler::renderUI()
{
	// ImGui UI rendering
	if (ImGui::Begin("Objects")) {
		// Tabs
		if (ImGui::BeginTabBar("Tabs")) {
			// Tab 1
			if (ImGui::BeginTabItem("Objects")) {
				ImGui::Text("Objects");
				ImGui::Separator();
		
				for (size_t i = 0; i < objects.size(); ++i) {
					if (ImGui::Selectable(std::string("Object " + std::to_string(i)).c_str())) {
						activeObject = objects[i];
					}
				}
				//Position
				if (activeObject != nullptr)
				{
					ImGui::Text("Position");
					ImGui::Separator();
					float* pos = &activeObject->getPosition().x;
					ImGui::SliderFloat3("Position", pos, -100, 100);
		
					ImGui::Separator();
		
					ImGui::Text("Rotation");
					ImGui::Separator();
					float* rot = &activeObject->getRotation().x;
					ImGui::SliderFloat3("Rotation", rot, -360, 360);
		
					ImGui::Separator();
		
					ImGui::Text("Scale");
					ImGui::Separator();
					float* sca = &activeObject->getScale().x;
					ImGui::SliderFloat3("Scale", sca, 0, 100);
				}
		
		
				ImGui::EndTabItem();
			}
		
			// Tab 2
			if (ImGui::BeginTabItem("Lights")) {
				ImGui::Text("Lights");
				ImGui::Separator();
		
				for (size_t i = 0; i < lights.size(); ++i) {
					if (ImGui::Selectable(std::string("Light " + std::to_string(i)).c_str())) {
						activeLight = lights[i];
					}
				}
		
				if (activeLight != nullptr)
				{
					DirectX::XMFLOAT4 tempColor = activeLight->getColor();
					ImGui::ColorPicker4("Color##4", (float*)&tempColor);
					activeLight->setColor(tempColor);
		
					ImGui::Separator();
		
					ImGui::Text("Position");
					ImGui::Separator();
					DirectX::XMFLOAT3 tempPos = activeLight->getPosition();
					ImGui::SliderFloat3("Position", (float*)&tempPos, -100, 100);
					activeLight->setPosition(tempPos);
		
					ImGui::Separator();
		
					//Check if it has a direction
					if (activeLight->getLightType() != LightType::PointLight_E)
					{
						ImGui::Text("Rotation");
						ImGui::Separator();
						DirectX::XMFLOAT3 tempRot = activeLight->getRotation();
						ImGui::SliderFloat3("Rotation", (float*)&tempRot, -360, 360);
						activeLight->setRotation(tempRot);
					}
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void IMGUIHandler::renderUIFromScene()
{
	sceneManager->getCurrentScene()->RenderUI();
}

void IMGUIHandler::addObject(Object* object)
{
	this->objects.push_back(object);
}

void IMGUIHandler::addLight(Light* light)
{
	this->lights.push_back(light);
}
