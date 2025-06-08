#pragma once
#include "../hlp_state.hh"

namespace hlp::d12
{
	U0 InitD3D12(FAppState* pAS);
	U0 DrawD3D12(FAppState* pAS);
	U0 CleanUpD3D12(FAppState* pAS);
}