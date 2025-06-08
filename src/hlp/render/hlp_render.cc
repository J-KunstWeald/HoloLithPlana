#include "hlp_d3d11.hh"
#include "hlp_d3d12.hh"

namespace hlp
{

	U0 InitD3D(FAppState* pAS)
	{
#if HLP_D3D12
		d12::InitD3D12(pAS);
#else
		d11::InitD3D11(pAS);
#endif
	}

	U0 DrawD3D(FAppState* pAS)
	{
#if HLP_D3D12
		d12::DrawD3D12(pAS);
#else
		d11::DrawD3D11(pAS);
#endif
	}

	U0 CleanUpD3D(FAppState* pAS)
	{
#if HLP_D3D12
		d12::CleanUpD3D12(pAS);
#else
		d11::CleanUpD3D11();
#endif
	}

}