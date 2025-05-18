#include <windows.h>
#include <stdexcept>
#include <iostream>
#include "ModelToVoxel.h"
#include "VoxelScene.h"
#include <dxgidebug.h>

void ReportLiveObjects()
{
	// Obtain the DXGI Debug Interface
	IDXGIDebug1* dxgiDebug = nullptr;
	HRESULT hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
	if (SUCCEEDED(hr))
	{
		// Report live objects
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		dxgiDebug->Release();
	}
	else
	{
		std::cerr << "Failed to get DXGI Debug Interface." << std::endl;
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	{
		Scene* ModelScene = new ModelToVoxel();//IS DELETED IN SCENEHANDLER
		//Scene* Voxelscene = new VoxelScene();//IS DELETED IN SCENEHANDLER
		Engine engine(ModelScene);
		engine.start();
	}
	//ReportLiveObjects();//FOR SOME FUCKING REASON MAIN_DEVICE IS STILL LIVE!

	return 0;
}