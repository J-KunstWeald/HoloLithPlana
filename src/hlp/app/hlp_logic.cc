#include "hlp_logic.hh"
#include "../hlp_math.hh"

namespace hlp
{
	F32 g_MoveLeftRight;
	F32 g_MoveBackForward;
	F32 g_MoveDownUp;

	F32 g_CamYaw;
	F32 g_CamPitch;

	U0 UpdateCamera(FAppState* pAppState)
	{
		// QWEASD
		// front/back
		if (pAppState->pInputState->bMoveForward == pAppState->pInputState->bMoveBackward)
		{
			g_MoveBackForward = 0.0f;
		}
		else
		{
			if (pAppState->pInputState->bMoveForward)
			{
				g_MoveBackForward += pAppState->CamSpeed * pAppState->DeltaTime;
			}
			else if (pAppState->pInputState->bMoveBackward)
			{
				g_MoveBackForward -= pAppState->CamSpeed * pAppState->DeltaTime;
			}
		}

		if (pAppState->pInputState->bMoveLeft == pAppState->pInputState->bMoveRight)
		{
			g_MoveLeftRight = 0.0f;
		}
		else
		{
			if (pAppState->pInputState->bMoveLeft)
			{
				g_MoveLeftRight += pAppState->CamSpeed * pAppState->DeltaTime;
			}
			else if (pAppState->pInputState->bMoveRight)
			{
				g_MoveLeftRight -= pAppState->CamSpeed * pAppState->DeltaTime;
			}
		}

		if (pAppState->pInputState->bMoveUp == pAppState->pInputState->bMoveDown)
		{
			g_MoveDownUp = 0.0f;
		}
		else
		{
			if (pAppState->pInputState->bMoveUp)
			{
				g_MoveDownUp += pAppState->CamSpeed * pAppState->DeltaTime;
			}
			else if (pAppState->pInputState->bMoveDown)
			{
				g_MoveDownUp -= pAppState->CamSpeed * pAppState->DeltaTime;
			}
		}

		pAppState->pSceneState->Cam.CameraPosition = pAppState->pSceneState->Cam.CameraPosition + (g_MoveLeftRight * pAppState->DefaultRight);
		pAppState->pSceneState->Cam.CameraPosition = pAppState->pSceneState->Cam.CameraPosition + (g_MoveBackForward * pAppState->DefaultFwd);
		pAppState->pSceneState->Cam.CameraPosition = pAppState->pSceneState->Cam.CameraPosition + (g_MoveDownUp * pAppState->DefaultUp);

		//pAppState->pSceneState->Cam.CameraForward = Normalize(pAppState->pSceneState->Cam.CameraForward);

		pAppState->pSceneState->Cam.CameraForward = pAppState->pSceneState->Cam.CameraForward + pAppState->pSceneState->Cam.CameraPosition;

		g_MoveBackForward = 0.0f;
		g_MoveLeftRight = 0.0f;
		g_MoveDownUp = 0.0f;
	}

	U0 UpdateState(FAppState* pAppState)
	{
		UpdateCamera(pAppState);
	}
}