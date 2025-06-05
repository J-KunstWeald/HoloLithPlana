#include <windows.h>
#include "hlp_defines.hh"
#include "hlp_util.hh"
#include "hlp_gamelogic.hh"
#include "stdio.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"


namespace hlp
{
    void ShowInputWindow(FCamera* camera)
    {
        static char inputBuffer[128] = "";

        ImGui::Begin("Input Window");;

        ImGui::InputFloat("Cam Fwd X:", &(camera->CameraForward.x));
        ImGui::InputFloat("Cam Fwd Y:", &(camera->CameraForward.y));
        ImGui::InputFloat("Cam Fwd Z:", &(camera->CameraForward.z));

        ImGui::InputFloat("Cam Pos X:", &(camera->CameraPosition.x));
        ImGui::InputFloat("Cam Pos Y:", &(camera->CameraPosition.x));
        ImGui::InputFloat("Cam Pos Z:", &(camera->CameraPosition.x));

        ImGui::InputFloat("FOV:", &(camera->HorizontalFOVDeg));

        //ImGui::Checkbox();

        ImGui::Text("Current Pos: ( %f, %f, %f, )", camera->CameraPosition.x, camera->CameraPosition.y, camera->CameraPosition.z);
        ImGui::Text("Current Fwd: ( %f, %f, %f, )", camera->CameraForward.x, camera->CameraForward.y, camera->CameraForward.z);
        ImGui::Text("Current FOV: %f", camera->HorizontalFOVDeg);

        ImGui::End();
    }

    void PrintResources(FResource* pResources, FRenderState* pRenderState)
    {
        printf("NumberOfResources: %u\n", pRenderState->NumResources);
        for (int i = 0; i < pRenderState->NumResources; i++)
        {
            if (pResources[i].Type == MESH)
            {
                printf("Is MESH, index: %d\n", i);
                printf("NumInd: %u\n", pResources->pData.pMesh->NumIdx);
                printf("NumVerts: %u\n", pResources->pData.pMesh->NumVerts);
            }
            else if (pResources[i].Type == POINTCLOUD)
            {
                printf("Is POINTCLOUD, index %d\n",i);
                printf("NumInd: %u\n", pResources->pData.pPointcloud->NumVerts);
            }
        }
    }
}