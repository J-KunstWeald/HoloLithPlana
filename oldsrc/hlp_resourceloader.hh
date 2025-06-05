#pragma once
#include "hlp_geometry.hh"
#include "hlp_core.hh"

namespace hlp
{
	struct Float3List
	{
		float3* Verts;
		U32 Length;
	};
	Float3List LoadFloat3List(const char* filename);
	//SpatialGeometry LoadFloat3List(const char* filename);
	//void ExportFloat3List(SpatialGeometry pcl, char const* name);
	FResource* LoadResources(FRenderState* pRenderState);
	U0 CleanUpResources(FResource* pResources, FRenderState* pRenderState);
}