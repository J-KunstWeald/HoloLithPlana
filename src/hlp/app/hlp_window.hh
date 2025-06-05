#pragma once
#include "../hlp_defines.hh"
#include "../hlp_state.hh"

namespace hlp
{
	U0 InitWin32Window(FAppState* pAppState);
	U32 HandleMsg(FAppState* pAppState);
}