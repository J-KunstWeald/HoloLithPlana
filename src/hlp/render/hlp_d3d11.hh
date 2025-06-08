#pragma once
#include "../hlp_state.hh"

namespace hlp::d11
{
	U0 InitD3D11(FAppState* pAppState);
	U0 DrawD3D11(FAppState* pAppState);
	U0 CleanUpD3D11(U0);
}