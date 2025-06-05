#include "stdio.h"
#include "hlp_core.hh"
#include "hlp_windows.hh"
#include "hlp_rendering.hh"
#include "hlp_resourceloader.hh"

#include "hlp_util.hh"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

namespace hlp
{
#if HLP_DB_WIN32_FILE
#else
	// this is the main message handler for the program
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		//	return S_OK;

		FInputState* pInput = reinterpret_cast<FInputState*>(
			GetWindowLongPtr(hWnd, GWLP_USERDATA));

		// sort through and find what code to run for the message given
		switch (message)
		{
			// this message is read when the window is closed
		case WM_DESTROY:
		{
			// close the application entirely
			PostQuitMessage(0);
			return 0;
		} break;
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				OutputDebugStringA("Escape pressed - exiting.\n");
				PostQuitMessage(0);
			}
			else
			{
				switch (wParam) {
				case 'W':
					pInput->bMoveForward = true;
					break;
				case 'S':
					pInput->bMoveBackward = true;
					break;
				case 'A':
					pInput->bMoveLeft = true;
					break;
				case 'D':
					pInput->bMoveRight = true;
					break;
				case 'Q':
					pInput->bMoveUp = true;
					break;
				case 'E':
					pInput->bMoveDown = true;
					break;
				}break;
			}
		}break;
		case WM_KEYUP:
		{
			switch (wParam)
			{
			case 'W':
				pInput->bMoveForward = false;
				break;
			case 'A':
				pInput->bMoveLeft = false;
				break;
			case 'S':
				pInput->bMoveBackward = false;
				break;
			case 'D':
				pInput->bMoveRight = false;
				break;
			case 'Q':
				pInput->bMoveUp = false;
				break;
			case 'E':
				pInput->bMoveDown = false;
				break;
			}
			break;
		} break;
		case WM_RBUTTONDOWN:
		{
			pInput->bMouseLock = !(pInput->bMouseLock);
		}break;
		case WM_MOUSEMOVE:
		{
			if (pInput->bMouseLock)
			{
				pInput->MouseX = GET_X_LPARAM(lParam);
				pInput->MouseY = GET_Y_LPARAM(lParam);
			}



		}break;
		default:
		{
			// Handle any messages the switch statement didn't
			return DefWindowProc(hWnd, message, wParam, lParam);
		}break;


		}
	}

	void InitWin32Window(HWND* hWnd, FRenderState* pRenderState)
	{
		// this struct holds information for the window class
		WNDCLASSEX wc;

		// clear out the window class for use
		ZeroMemory(&wc, sizeof(WNDCLASSEX));

		// fill in the struct with the needed information
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		//wc.hbrBackground = (HBRUSH)COLOR_WINDOW; // removed to make window invisible upon startup
		wc.lpszClassName = L"Printf_3dWindowClass";

		// register the window class
		RegisterClassEx(&wc);

		RECT wr = { 0, 0, 500, 400 };    // set the size, but not the position
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);    // adjust the size

		// create the window and use the result as the handle
		*hWnd = CreateWindowEx(NULL,
			L"Printf_3dWindowClass",    // name of the window class
			L"Printf_3d",   // title of the window
			WS_OVERLAPPEDWINDOW,    // window style
			300,    // x-position of the window
			300,    // y-position of the window
			pRenderState->Width,    // width of the window
			pRenderState->Height,    // height of the window
			NULL,    // we have no parent window, NULL
			NULL,    // we aren't using menus, NULL
			GetModuleHandle(NULL),    // application handle
			NULL);    // used with multiple windows, NULL

		// display the window on the screen
		ShowWindow(*hWnd, 1);

	}

	U32 HandleMsg(MSG* pMsg)
	{
		// Check to see if any messages are waiting in the queue
		if (PeekMessage(pMsg, NULL, 0, 0, PM_REMOVE))
		{
			// translate keystroke messages into the right format
			TranslateMessage(pMsg);

			// send the message to the WindowProc function
			DispatchMessage(pMsg);

			// check to see if it's time to quit
			if (pMsg->message == WM_QUIT)
				return 1;
		}
		else
		{
			// Run game code here
			// ...
			// ...
		}
		return 0;
	}
#endif
	// Default init values
	
	// camera
#define HLP_DEFAULT_CAM_POS float3{0.f,0.f,-4.0f}
#define HLP_DEFAULT_CAM_FWD float3{0.001,0.001,0.001}
#define HLP_DEFAULT_CAM_UP float3{0.,1.,0.}
#define HLP_DEFAULT_ASPECT_RATIO (16.f / 9.f)
#define HLP_DEFAULT_HFOV 90.f
#define HLP_DEFAULT_NEAR_PLANE 0.001f
#define HLP_DEFAULT_FAR_PLANE 1000.f
#define HLP_DEFAULT_AZIMUTH 0.
#define HLP_DEFAULT_ELEVATION 0. 

#define HLP_DEFAULT_CAM_SPEED 2.5f
#define HLP_DEFAULT_MOUSE_SENS 0.001f

#define HLP_DEFAULT_WIDTH 1920
#define HLP_DEFAULT_HEIGHT 1080
#define HLP_DEFAULT_FPS 60
	// Frame counting/delta time
	//U64 g_TimerFrequency = 0;

	U0 Init(FGameLogicState* pGameLogicState, FInputState* pInputState, FRenderState* pRenderState)
	{
		// Init default state
		pGameLogicState->Cam.CameraPosition = HLP_DEFAULT_CAM_POS;
		pGameLogicState->Cam.CameraForward = HLP_DEFAULT_CAM_FWD;
		pGameLogicState->Cam.CameraUp = HLP_DEFAULT_CAM_UP;
		pGameLogicState->Cam.AspectRatio = HLP_DEFAULT_ASPECT_RATIO;
		pGameLogicState->Cam.HorizontalFOVDeg = HLP_DEFAULT_HFOV;
		pGameLogicState->Cam.NearPlane = HLP_DEFAULT_NEAR_PLANE;
		pGameLogicState->Cam.FarPlane = HLP_DEFAULT_FAR_PLANE;
		pGameLogicState->Cam.Azimuth = HLP_DEFAULT_AZIMUTH;
		pGameLogicState->Cam.Elevation = HLP_DEFAULT_ELEVATION;

		pGameLogicState->CamSpeed = HLP_DEFAULT_CAM_SPEED;
		pGameLogicState->MouseSens = HLP_DEFAULT_MOUSE_SENS;

		pRenderState->NumResources = 1;
		pRenderState->Height = HLP_DEFAULT_HEIGHT;
		pRenderState->Width = HLP_DEFAULT_WIDTH;
		pRenderState->MaxFPS = HLP_DEFAULT_FPS;
		pRenderState->NumIBuf = 0;
		pRenderState->NumVBuf = 0;
		pRenderState->TimerFrequency = 0;
	}

	U0 Run()
	{
		// Init phase
		FGameLogicState GameLogicState = { };
		FInputState InputState = { };
		FRenderState RenderState = { };
		HWND hWnd;
		MSG msg = { };

		Init(&GameLogicState, &InputState, &RenderState);
		printf("Init Done\n");
		
		//FResource* pResources = LoadResources(&RenderState);
		//PrintResources(pResources, &RenderState);
		//printf("Load Resources Done\n");
		
		FResource* pResources = (FResource*)malloc(2 * sizeof(FResource));
		Float3List Ghost90 = LoadFloat3List("../resources/geometry/ghost_pcl.txt");

		FRenderMesh MeshGhost90 = { };
		MeshGhost90.NumVerts = Ghost90.Length;
		MeshGhost90.NumIdx = Ghost90.Length;
		MeshGhost90.Verts = (float3*)malloc(MeshGhost90.NumVerts * sizeof(float3));
		MeshGhost90.Idx = (U32*)malloc(MeshGhost90.NumIdx * sizeof(U32));

		for (int i = 0; i < MeshGhost90.NumVerts; i++)
		{
			MeshGhost90.Verts[i] = Ghost90.Verts[i];
			MeshGhost90.Idx[i] = i;
		}
		free(Ghost90.Verts);

		Float3List Ghost = LoadFloat3List("../resources/geometry/ghost_pcl.txt");
		FRenderPointcloud PclGhost = { };
		PclGhost.NumVerts = Ghost.Length;
		PclGhost.Verts = (float3*)malloc(PclGhost.NumVerts * sizeof(float3));

		for (int i = 0; i < PclGhost.NumVerts; i++)
		{
			PclGhost.Verts[i] = Ghost.Verts[i];
		}

		free(Ghost.Verts);
		PrintResources(pResources, &RenderState);

		pResources[0] = FResource{ &MeshGhost90,MESH };
		pResources[1] = FResource{ NULL, POINTCLOUD };
		pResources[1].pData.pPointcloud = &PclGhost;

		InitWin32Window(&hWnd, &RenderState);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&InputState));
		PrintResources(pResources, &RenderState);
		printf("Win Init Done\n");
		
		InitD3D(&hWnd, &RenderState, pResources);
		PrintResources(pResources, &RenderState);
		printf("D3D Init Done\n");
		
		UploadVbuf(pResources, &RenderState);
		printf("VBuf Up Done\n");
		
		UploadIBuf(pResources, &RenderState);
		printf("IBuf Up Done\n");
		// init timer
		U64 PerfCounter = 0;
		LARGE_INTEGER PerfCounterWin32 = {};
		QueryPerformanceCounter(&PerfCounterWin32);
		PerfCounter = (U64)PerfCounterWin32.QuadPart;

		// Main loop
		while (true)
		{
			// Query FPS
			U64 const PrevFrameCounter = PerfCounter;

			QueryPerformanceCounter(&PerfCounterWin32);
			PerfCounter = (U64)PerfCounterWin32.QuadPart;

			U64 const NumTicksPrevFrame = PerfCounter - PrevFrameCounter;

			F64 const DeltaTime = F64(NumTicksPrevFrame) / F64(RenderState.TimerFrequency);
			// Handle Gamelogic

			// upload cbuf
			UploadCbuf(&GameLogicState);
			
			// imgui newframe
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			
			ShowInputWindow(&(GameLogicState.Cam));

			if (HandleMsg(&msg))
				break;
				
			// printf FPS	
			//printf("Delta Time: %.2f ms (%.0f FPS)\n", DeltaTime * 1000, 1.0 / DeltaTime);

			// draw
			DrawFrame(pResources, &RenderState);
		}

		// Cleanup
		CleanUpD3D();
		CleanUpResources(pResources, &RenderState);

	}
}