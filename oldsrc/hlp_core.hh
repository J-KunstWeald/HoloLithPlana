#pragma once
#include "hlp_defines.hh"
#include "hlp_math.hh"

namespace hlp
{
	struct FCamera
	{
		float3 CameraPosition;
		float3 CameraForward;
		float3 CameraUp;
		F32 AspectRatio;
		F32 HorizontalFOVDeg;
		F32 NearPlane;
		F32 FarPlane;
		F32 Azimuth;
		F32 Elevation;
	};

	struct FGameLogicState
	{
		FCamera Cam;
		F32 CamSpeed;
		F32 MouseSens;

	};

	struct FInputState
	{
		bool bMoveForward;
		bool bMoveBackward;
		bool bMoveUp;
		bool bMoveDown;
		bool bMoveLeft;
		bool bMoveRight;
		// mouse
		F32 LastMouseX;
		F32 LastMouseY;
		F32 MouseX;
		F32 MouseY;
		bool bMouseLock;
	};

	struct FRenderState
	{
		F32 NumResources;
		F32 Height;
		F32 Width;
		I32 MaxFPS;
		U32 NumIBuf;
		U32 NumVBuf;
		U64 TimerFrequency;

	};

	struct FRenderMesh
	{
		float3* Verts;
		U32* Idx;
		U32 NumVerts;
		U32 NumIdx;
	};

	struct FRenderPointcloud
	{
		float3* Verts;
		U32 NumVerts;
	};

	enum ResourceType
	{
		MESH,
		POINTCLOUD
	};

	union SpatialGeometry
	{
		FRenderMesh* pMesh;
		FRenderPointcloud* pPointcloud;
	};

	struct FResource
	{
		SpatialGeometry pData;
		ResourceType Type;
	};

	U0 Run();
}