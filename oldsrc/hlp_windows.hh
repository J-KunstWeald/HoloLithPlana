#pragma once
#define UNICODE 1
#include <windows.h>
#include <windowsx.h>
#include "hlp_core.hh"

namespace hlp
{
	void InitWin32Window(HWND* hWnd, FRenderState* pRenderState);
	U32 HandleMsg(MSG* pMsg);
}