#pragma once
#include "hlp_state.hh"
#include "hlp_defines.hh"
#include "hlp_math.hh"

namespace hlp
{
	//struct Mesh
	//{
	//	float3* Verts;
	//	U32 Length;
	//};

	//struct Pointcloud
	//{
	//	float3* Verts;
	//	U32 Length;
	//};

	//enum GeoType
	//{
	//	MESH,
	//	POINTCLOUD
	//};

	//struct SpatialGeometry
	//{
	//	float3* Verts;
	//	U32 Length;
	//	GeoType type;
	//};

	U0 GetIndexMesh(float3* pFloatList, U32 Length, FMesh* pOut);

}