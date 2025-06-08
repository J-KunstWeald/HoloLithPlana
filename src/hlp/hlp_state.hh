#pragma once
//#define WIN32_LEAN_AND_MEAN
#define UNICODE 1
#include "hlp_defines.hh"
#include <Windows.h>

namespace hlp
{
	struct FMesh
	{
		float3* Verts;
		float3* Normals; // VertNormals
		U32* Idx;
		U32 NumVerts;
		U32 NumIdx;
		float4x4 World;
		float4 Color;
	};

	struct FPointcloud
	{
		float3* Verts;
		U32 NumVerts;
		float4x4 World;
	};

	union USpatialGeometryData
	{
		FMesh* pMesh;
		FPointcloud* pPointcloud;
		U32 GeoId;
	};

	enum ESpatialGeometryType
	{
		MESH,
		POINTCLOUD
	};

	struct FSpatialGeometry
	{
		USpatialGeometryData pData;
		ESpatialGeometryType Type;

	};

	struct FPointLight
	{

	};

	struct FDirLight
	{

	};

	struct FSpotLight
	{

	};

	union ULightData
	{
		FPointLight* pPointLight;
		FSpotLight* pSpotLight;
		FDirLight* pDirLight;

	};

	enum ELightType
	{
		POINTLIGHT,
		SPOTLIGHT,
		DIRLIGHT
	};

	struct FLight
	{
		ULightData pData;
		ELightType Type;
	};

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

	struct FSceneState
	{
		FCamera Cam;
		FLight* Lights;
		FSpatialGeometry* Geometries;
		U32 NumLights;
		U32 NumGeometries;
		U32 NextGeoId;

	};

	struct FRenderState
	{
		bool bNewGeometry;
		bool bNewLight;
		bool bChangeRasterizer;
		bool bRenderSolid;
		bool bRenderWireframe;
		I32 MaxFps;
		I32 TotNumIdx;
		I32 TotNumVerts;
		I32 NumFrames;
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
		F32 DeltaX;
		F32 DeltaY;
	};

	struct FAppState
	{
		HWND hWnd;
		MSG Msg;
		HINSTANCE hInstance;
		FRenderState* pRenderState;
		FSceneState* pSceneState;
		FInputState* pInputState;
		U64 TimerFrequency;
		U32 DeltaTime;
		I32 Width;
		I32 Height;
		I32 ShowWnd;
		F32 CamSpeed;
		F32 MouseSens;
		float3 DefaultFwd;
		float3 DefaultRight;
		float3 DefaultUp;
	};

	enum EFileType
	{
		TXT_MESH,
		TXT_PCL
	};
}