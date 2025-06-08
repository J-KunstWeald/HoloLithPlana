#include "hlp_app.hh"
#include "hlp_window.hh"
#include "../render/hlp_render.hh"
#include "hlp_resource.hh"
#include "hlp_init.hh"
#include <malloc.h>
#include "hlp_logic.hh"
#include "hlp_util.hh"
#include "../render/hlp_imgui.hh"

namespace hlp
{
	FAppState* g_pAppState;

	U0 InitRenderState()
	{
		g_pAppState->pRenderState->bNewGeometry = true;
		g_pAppState->pRenderState->bNewLight = true;
		g_pAppState->pRenderState->NumFrames = HLP_DEFAULT_D12_NUMFRAMES;
	}

	U0 InitInputState()
	{

	}

	U0 InitCamera()
	{
		g_pAppState->pSceneState->Cam.CameraPosition = HLP_DEFAULT_CAM_POS;
		g_pAppState->pSceneState->Cam.CameraForward = HLP_DEFAULT_CAM_FWD;
		g_pAppState->pSceneState->Cam.CameraUp = HLP_DEFAULT_CAM_UP;
		g_pAppState->pSceneState->Cam.AspectRatio = HLP_DEFAULT_ASPECT_RATIO;
		g_pAppState->pSceneState->Cam.HorizontalFOVDeg = HLP_DEFAULT_HFOV;
		g_pAppState->pSceneState->Cam.NearPlane = HLP_DEFAULT_NEAR_PLANE;
		g_pAppState->pSceneState->Cam.FarPlane = HLP_DEFAULT_FAR_PLANE;
		g_pAppState->pSceneState->Cam.Azimuth = HLP_DEFAULT_AZIMUTH;
		g_pAppState->pSceneState->Cam.Elevation = HLP_DEFAULT_ELEVATION;
	}

	U0 InitApp()
	{
		// Malloc the app
		g_pAppState = (FAppState*)malloc(sizeof(FAppState));
		HLP_ASSERT(g_pAppState);
		ZeroMemory(g_pAppState, sizeof(FAppState));
		
		// Set module handle
		g_pAppState->hInstance = GetModuleHandle(NULL);

		// Set clock freq
		LARGE_INTEGER TimerFrequency;
		QueryPerformanceFrequency(&TimerFrequency);
		g_pAppState->TimerFrequency = (U64)TimerFrequency.QuadPart;

		// Malloc RenderState, SceneState, InputState
		g_pAppState->pInputState = (FInputState*)malloc(sizeof(FInputState));
		HLP_ASSERT(g_pAppState->pInputState);
		ZeroMemory(g_pAppState->pInputState, sizeof(FInputState));

		g_pAppState->pRenderState = (FRenderState*)malloc(sizeof(FRenderState));
		HLP_ASSERT(g_pAppState->pRenderState);
		ZeroMemory(g_pAppState->pRenderState, sizeof(FRenderState));

		g_pAppState->pSceneState = (FSceneState*)malloc(sizeof(FSceneState));
		HLP_ASSERT(g_pAppState->pRenderState);
		ZeroMemory(g_pAppState->pSceneState, sizeof(FSceneState));


		g_pAppState->Width = HLP_DEFAULT_WIDTH;
		g_pAppState->Height = HLP_DEFAULT_HEIGHT;
		g_pAppState->ShowWnd = HLP_DEFAULT_SHOW_WND;
		g_pAppState->CamSpeed = HLP_DEFAULT_CAM_SPEED;
		g_pAppState->DefaultFwd = HLP_DEFAULT_DEFAULT_FWD;
		g_pAppState->DefaultRight = HLP_DEFAULT_DEFAULT_RIGHT;
		g_pAppState->DefaultUp = HLP_DEFAULT_DEFAULT_UP;
		g_pAppState->DeltaTime = HLP_DEFAULT_DTIME;				
		g_pAppState->MouseSens = HLP_DEFAULT_MOUSE_SENS;

		LoadResources(g_pAppState);
		InitRenderState();
		InitCamera();
	}



	U0 Init()
	{
		InitApp();
		InitWin32Window(g_pAppState);
		InitD3D(g_pAppState);
	}

	U0 CleanUpApp()
	{
		// Clean all geometries

		// Clean geo arr

		// Clean all lights

		// Clean lights arr
		CleanUpScene(g_pAppState);
		// Clean SceneState 
		
		// Clean InputState
		free(g_pAppState->pInputState);
		// Clean RenderState
		free(g_pAppState->pRenderState);
		// Clean Appstate
		free(g_pAppState);
	}

	U0 CleanUp()
	{
		CleanUpD3D(g_pAppState);
		CleanUpApp();
	}

	U0 Run()
	{
		Init();
		PrintCam(g_pAppState->pSceneState->Cam);
		while (true)
		{
			UpdateState(g_pAppState);
			//PrintCam(g_pAppState->pSceneState->Cam);
			if (HandleMsg(g_pAppState))
			{
				break;
			}
//			PrintCam(g_pAppState->pSceneState->Cam);
		}

		CleanUp();
	}




}