#include "hlp_resourceloader.hh"
#include <time.h>
#include <stdio.h>
#include <vector>

namespace hlp
{

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

	FResource* LoadResources(FRenderState* pRenderState)
	{
		// load raw data


		pRenderState->NumResources = 2;
		FResource* pResources = (FResource*)malloc(2 * sizeof(FResource));
		Float3List Ghost90 = LoadFloat3List("../resources/geometry/ghost_90_pcl.txt");
		
		FRenderMesh MeshGhost90 = { };
		MeshGhost90.NumVerts = Ghost90.Length;
		MeshGhost90.NumIdx = Ghost90.Length;
		MeshGhost90.Verts = (float3*)malloc(MeshGhost90.NumVerts * sizeof(float3));
		MeshGhost90.Idx = (U32*)malloc(MeshGhost90.NumIdx * sizeof(U32));

		for (int i = 0; i < MeshGhost90.NumVerts; i++)
		{
			MeshGhost90.Verts[i] = Ghost90.Verts[i];
			MeshGhost90.Idx[i] = i;
		}
		free(Ghost90.Verts);

		Float3List Ghost = LoadFloat3List("../resources/geometry/ghost_pcl.txt");
		FRenderPointcloud PclGhost = { };
		PclGhost.NumVerts = Ghost.Length;
		PclGhost.Verts = (float3*)malloc(PclGhost.NumVerts * sizeof(float3));

		for (int i = 0; i < PclGhost.NumVerts; i++)
		{
			PclGhost.Verts[i] = Ghost.Verts[i];
		}

		free(Ghost.Verts);
		

		pResources[0] = FResource{&MeshGhost90,MESH};
		pResources[1] = FResource{NULL, POINTCLOUD };
		pResources[1].pData.pPointcloud = &PclGhost;

		return pResources;
	}

	U0 CleanUpResources(FResource* pResources, FRenderState* pRenderState)
	{
		for (int i = 0; i < pRenderState->NumResources; i++)
		{
			if (pResources[i].Type == MESH)
			{
				free(pResources[i].pData.pMesh->Verts);
				free(pResources[i].pData.pMesh->Idx);
			}
			else if (pResources[i].Type == POINTCLOUD)
			{
				free(pResources[i].pData.pPointcloud->Verts);
			}
		}
		free(pResources);
	}
}