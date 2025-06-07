#include "hlp_resource.hh"
#include <stdio.h>
#include <vector>

namespace hlp
{
	struct Float3List
	{
		float3* Verts;
		U32 Length;
	};

	void serializeFloat(FILE* pFile, F32 value)
	{
		U32 i = *reinterpret_cast<U32*>(&value);
		fprintf(pFile, "%08x", i);
	}

	void ExportFloat3List(Float3List pcl, char const* name)
	{
		FILE* pFileOut = fopen(name, "w");
		serializeFloat(pFileOut, 1.0f);
		fprintf(pFileOut, "\n");
		for (int i = 0; i < pcl.Length; i++)
		{
			serializeFloat(pFileOut, pcl.Verts[i].x);
			serializeFloat(pFileOut, pcl.Verts[i].y);
			serializeFloat(pFileOut, pcl.Verts[i].z);
		}
		fprintf(pFileOut, "\n");
		fclose(pFileOut);
	}

	float deserializeFloat(FILE* pFile)
	{
		U32 i;
		fscanf(pFile, "%08x", &i);
		return *reinterpret_cast<F32*>(&i);
	}

	Float3List LoadFloat3List(const char* filename)
	{
		std::vector<float3> res;
		FILE* pFileIn = fopen(filename, "r");
		// big endian check
		F32 endianCheck = deserializeFloat(pFileIn);
		fgetc(pFileIn);
		if (endianCheck != 1.0f)
		{
			printf("\n!!!ENDIAN ERROR!!!\n");
		}
		int c;
		while ((c = fgetc(pFileIn)) != EOF) // end of file
		{
			ungetc(c, pFileIn);
			while ((c = fgetc(pFileIn)) != '\n') // end of face
			{
				ungetc(c, pFileIn);
				F32 x = deserializeFloat(pFileIn);
				F32 y = deserializeFloat(pFileIn);
				F32 z = deserializeFloat(pFileIn);
				res.push_back(float3{ x, y, z });
				//printf("%f, %f, %f %ld\n", x, y, z, ftell(pFileIn));
			}

		}
		fclose(pFileIn);

		Float3List pcl = { };
		pcl.Length = res.size();
		printf("Float3 list load complete, list contains %d float3s\n", pcl.Length);
		pcl.Verts = (float3*)malloc(pcl.Length * sizeof(float3));
		for (I32 i = 0; i < pcl.Length; i++)
		{
			pcl.Verts[i] = res[i];
		}
		return pcl;
	}

	U0 LoadResources(FAppState* pAppState)
	{
		pAppState->pRenderState->bNewGeometry = true;
		U32 TotalNumberVerts = 0;
		U32 TotalNumberIndicies = 0;

		Float3List pMeshA = LoadFloat3List("../resources/geometry/OctoCat.txt");

		FMesh* pMeshGhost90 = (FMesh*)malloc(sizeof(FMesh));
		pMeshGhost90->NumVerts = pMeshA.Length;
		pMeshGhost90->NumIdx = pMeshA.Length;
		pMeshGhost90->Verts = (float3*)malloc(pMeshGhost90->NumVerts * sizeof(float3));
		pMeshGhost90->Normals = (float3*)malloc(pMeshGhost90->NumVerts * sizeof(float3));
		pMeshGhost90->Idx = (U32*)malloc(pMeshGhost90->NumIdx * sizeof(U32));
		pMeshGhost90->Color = float4{ 0.2f, 0.4f, 0.1f, 1.0f };

		for (int i = 0; i < pMeshGhost90->NumVerts; i++)
		{
			pMeshGhost90->Verts[i] = pMeshA.Verts[i];
			pMeshGhost90->Normals[i] = pMeshA.Verts[i];
			pMeshGhost90->Idx[i] = i;
		}

		TotalNumberVerts += pMeshA.Length;
		TotalNumberIndicies += pMeshA.Length;

		free(pMeshA.Verts);

		Float3List pMeshB = LoadFloat3List("../resources/geometry/Bunny.txt");

		FMesh* pMeshGhost = (FMesh*)malloc(sizeof(FMesh));
		pMeshGhost->NumVerts = pMeshB.Length;
		pMeshGhost->NumIdx = pMeshB.Length;
		pMeshGhost->Verts = (float3*)malloc(pMeshGhost->NumVerts * sizeof(float3));
		pMeshGhost->Normals = (float3*)malloc(pMeshGhost->NumVerts * sizeof(float3));
		pMeshGhost->Idx = (U32*)malloc(pMeshGhost->NumIdx * sizeof(U32));
		pMeshGhost->Color = float4{ 0.7f, 0.2f, 0.15f, 1.0f };

		for (int i = 0; i < pMeshGhost->NumVerts; i++)
		{
			pMeshGhost->Verts[i] = pMeshB.Verts[i];
			pMeshGhost->Normals[i] = pMeshB.Verts[i];
			pMeshGhost->Idx[i] = i;
		}
		TotalNumberVerts += pMeshB.Length;
		TotalNumberIndicies += pMeshB.Length;

		free(pMeshB.Verts);

		pAppState->pSceneState->NumGeometries = 2;
		pAppState->pSceneState->Geometries = (FSpatialGeometry*)malloc(sizeof(FSpatialGeometry) * 2);
		pAppState->pSceneState->Geometries[0] = FSpatialGeometry{ pMeshGhost, MESH };
		pAppState->pSceneState->Geometries[1] = FSpatialGeometry{ pMeshGhost90, MESH };
		pAppState->pRenderState->TotNumVerts = TotalNumberVerts;
		pAppState->pRenderState->TotNumIdx = TotalNumberIndicies;
		

	}


	U0 LoadResourcesBox(FAppState* pAppState)
	{
		// Hardcoded data

		float3 v[] =
		{
			float3{-1.0f, -1.0f, -1.0f },
			float3{-1.0f, +1.0f, -1.0f },
			float3{+1.0f, +1.0f, -1.0f },
			float3{+1.0f, -1.0f, -1.0f },
			float3{-1.0f, -1.0f, +1.0f },
			float3{-1.0f, +1.0f, +1.0f },
			float3{+1.0f, +1.0f, +1.0f },
			float3{+1.0f, -1.0f, +1.0f },
		};

		U32 indi[] = {
			// front face
			0, 1, 2,
			0, 2, 3,

			// back face
			4, 6, 5,
			4, 7, 6,

			// left face
			4, 5, 1,
			4, 1, 0,

			// right face
			3, 2, 6,
			3, 6, 7,

			// top face
			1, 5, 6,
			1, 6, 2,

			// bottom face
			4, 0, 3,
			4, 3, 7
		};


		pAppState->pRenderState->bNewGeometry = true;
		U32 TotalNumberVerts = 8;
		U32 TotalNumberIndicies = 36;



		//Float3List Ghost = LoadFloat3List("../resources/geometry/ghost_non_pcl.txt");

		FMesh* pMeshGhost = (FMesh*)malloc(sizeof(FMesh));
		pMeshGhost->NumVerts = 8;
		pMeshGhost->NumIdx = 36;
		pMeshGhost->Verts = (float3*)malloc(pMeshGhost->NumVerts * sizeof(float3));
		pMeshGhost->Normals = (float3*)malloc(pMeshGhost->NumVerts * sizeof(float3));
		pMeshGhost->Idx = (U32*)malloc(pMeshGhost->NumIdx * sizeof(U32));
		pMeshGhost->Color = float4{ 1.0f, 1.0f, 1.0f, 1.0f };

		for (int i = 0; i < pMeshGhost->NumVerts; i++)
		{
			pMeshGhost->Verts[i] = v[i];
			pMeshGhost->Normals[i] = v[i];
		}

		for (int i = 0; i < pMeshGhost->NumIdx; i++)
		{
			pMeshGhost->Idx[i] = indi[i];
		}


		pAppState->pSceneState->NumGeometries = 1;
		pAppState->pSceneState->Geometries = (FSpatialGeometry*)malloc(sizeof(FSpatialGeometry) * 1);
		pAppState->pSceneState->Geometries[0] = FSpatialGeometry{ pMeshGhost, MESH };
		

		pAppState->pRenderState->TotNumIdx = TotalNumberIndicies;
		pAppState->pRenderState->TotNumVerts = TotalNumberVerts;

	}


	U0 CleanUpMesh(FMesh* mesh)
	{
		free(mesh->Verts);
		free(mesh->Normals);
		free(mesh->Idx);
	}

	U0 CleanUpPointcloud(FPointcloud* pointcloud)
	{
		free(pointcloud->Verts);
	}

	U0 CleanUpGeometries(FAppState* pAppState)
	{
		for (int i = 0; i < pAppState->pSceneState->NumGeometries; i++)
		{
			if (pAppState->pSceneState->Geometries[i].Type == MESH)
			{
				CleanUpMesh(pAppState->pSceneState->Geometries[i].pData.pMesh);
				free(pAppState->pSceneState->Geometries[i].pData.pMesh);
			}
			else if (pAppState->pSceneState->Geometries[i].Type == POINTCLOUD)
			{
				CleanUpPointcloud(pAppState->pSceneState->Geometries[i].pData.pPointcloud);
				free(pAppState->pSceneState->Geometries[i].pData.pPointcloud);
			}
		}
		free(pAppState->pSceneState->Geometries);
	}

	U0 CleanUpLights(FAppState* pAppState)
	{
		for (int i = 0; i < pAppState->pSceneState->NumLights; i++)
		{
			if (pAppState->pSceneState->Lights[i].Type == POINTLIGHT)
			{
				free(pAppState->pSceneState->Lights[i].pData.pPointLight);
			}
			else if (pAppState->pSceneState->Lights[i].Type == SPOTLIGHT)
			{
				free(pAppState->pSceneState->Lights[i].pData.pSpotLight);
			}
			else if (pAppState->pSceneState->Lights[i].Type == DIRLIGHT)
			{
				free(pAppState->pSceneState->Lights[i].pData.pDirLight);
			}
		}
		free(pAppState->pSceneState->Lights);
	}

	U0 CleanUpScene(FAppState* pAppState)
	{
		CleanUpGeometries(pAppState);
		CleanUpLights(pAppState);
		free(pAppState->pSceneState);
	}

	U0 AddMesh(const char* Filename, ESpatialGeometryType Type, FAppState* pAS)
	{

	}
	U0 RemoveMesh(U32 MeshId, FAppState* pAS)
	{

	}
	U0 InitScene(FAppState* pAS)
	{

	}
}