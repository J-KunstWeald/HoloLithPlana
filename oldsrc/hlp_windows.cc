#include "hlp_windows.hh"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace hlp
{
#if HLP_DB_WIN32_FILE
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
		UpdateWindow(*hWnd);

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
}