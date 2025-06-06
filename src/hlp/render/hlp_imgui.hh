#pragma once
#include "../hlp_state.hh"

namespace hlp
{
	U0 InitImgui(U0* phWnd, U0* pDevice, U0* pDevCon);

	U0 NewImguiFrame(FAppState* pAppState);

	U0 DrawImguiFrame();

	U0 CleanUpImgui();
}