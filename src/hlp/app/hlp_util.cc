#include "hlp_util.hh"
#include <stdio.h>
namespace hlp
{
	U0 PrintFloat3(float3 f)
	{
		printf("( %f, %f, %f )", f.x, f.y, f.z);
	}
	
	U0 PrintFloat4(float4 f)
	{
		printf("( %f, %f, %f, %f )", f.x, f.y, f.z, f.w);
	}

	U0 PrintFMesh(FMesh* mesh)
	{
		printf("NumIdx: %d\n", mesh->NumIdx);
		printf("NumVerts: %d\n", mesh->NumVerts);
		printf("Color: ");
		PrintFloat4(mesh->Color);
		printf("\n");
		for (int i = 0; i < mesh->NumVerts; i++)
		{
			PrintFloat3(mesh->Verts[i]);
			printf("\n");
		}
		for (int i = 0; i < mesh->NumIdx; i++)
		{
			printf("%d\n", mesh->Idx[i]);
		}
	}

	U0 PrintCam(FCamera cam)
	{
		printf("CPos\n");
		PrintFloat3(cam.CameraPosition);
		printf("FwdPos\n");
		PrintFloat3(cam.CameraForward);
	}
}