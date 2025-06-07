#pragma once
#include "../hlp_state.hh"

namespace hlp
{
	U0 AddMesh(const char* Filename, ESpatialGeometryType Type, FAppState* pAS);
	U0 RemoveMesh(U32 MeshId, FAppState* pAS);
	U0 InitScene(FAppState* pAS);
	U0 LoadResources(FAppState* pAppState);
	U0 LoadResourcesBox(FAppState* pAppState);
	U0 CleanUpScene(FAppState* pAppState);
}