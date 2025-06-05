#include "hlp_defines.hh"
#include "hlp_core.hh"

namespace hlp
{
	U0 InitD3D(HWND* phWnd, FRenderState* pRenderState, FResource* pResources);
	U0 UploadIBuf(FResource* pResources, FRenderState* pRenderState);
	U0 UploadVbuf(FResource* pResources, FRenderState* pRenderState);
	U0 UploadCbuf(FGameLogicState* pGLS);
	U0 DrawFrame(FResource* pResources, FRenderState* pRenderState);
	U0 CleanUpD3D(U0);
}