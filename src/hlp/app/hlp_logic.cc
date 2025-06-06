#include "hlp_logic.hh"
#include "../hlp_math.hh"
#include <Directxmath.h>

namespace hlp
{
	F32 g_MoveLeftRight;
	F32 g_MoveBackForward;
	F32 g_MoveDownUp;
	DirectX::XMMATRIX g_CamRotation;

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
#if 0
		if ((pAppState->pInputState->MouseX == pAppState->pInputState->LastMouseX) || (pAppState->pInputState->MouseY == pAppState->pInputState->LastMouseY))
		{
			pAppState->pSceneState->Cam.Azimuth += (pAppState->pInputState->MouseX - pAppState->pInputState->LastMouseX)* pAppState->MouseSens;
			pAppState->pSceneState->Cam.Elevation += (pAppState->pInputState->MouseY - pAppState->pInputState->LastMouseY) * pAppState->MouseSens;

			pAppState->pInputState->LastMouseX = pAppState->pInputState->MouseX;
			pAppState->pInputState->LastMouseY = pAppState->pInputState->MouseY;
		}
#else
		if (pAppState->pInputState->bMouseLock)
		{
			POINT center;
			center.x = 1080 / 2;
			center.y = 1920 / 2;

			// Convert to screen coords for SetCursorPos
			POINT screenCenter;
			screenCenter = center;
			ClientToScreen(pAppState->hWnd, &screenCenter);

			// Get mouse position
			POINT mousePos;
			GetCursorPos(&mousePos);

			// Compute delta
			int deltaX = mousePos.x - screenCenter.x;
			int deltaY = mousePos.y - screenCenter.y;

			// Apply to camera
			pAppState->pSceneState->Cam.Azimuth += deltaX * 0.002f; // tune sensitivity
			pAppState->pSceneState->Cam.Elevation += deltaY * 0.002f;

			// Clamp pitch
			pAppState->pSceneState->Cam.Elevation = max(-DirectX::XM_PIDIV2 + 0.01f, min(DirectX::XM_PIDIV2 - 0.01f, pAppState->pSceneState->Cam.Elevation));

			// Reset mouse to center
			SetCursorPos(screenCenter.x, screenCenter.y);
		}
#endif

#if 1
		//pAppState->pSceneState->Cam.CameraPosition = pAppState->pSceneState->Cam.CameraPosition + (g_MoveLeftRight * pAppState->DefaultRight);
		//pAppState->pSceneState->Cam.CameraPosition = pAppState->pSceneState->Cam.CameraPosition + (g_MoveBackForward * pAppState->DefaultFwd);
		//pAppState->pSceneState->Cam.CameraPosition = pAppState->pSceneState->Cam.CameraPosition + (g_MoveDownUp * pAppState->DefaultUp);
		// Full pitch + yaw rotation matrix
		g_CamRotation = DirectX::XMMatrixRotationRollPitchYaw(
			pAppState->pSceneState->Cam.Elevation,
			pAppState->pSceneState->Cam.Azimuth,
			0.0f);

		// Rotate default axes with full rotation
		DirectX::XMVECTOR camForward = DirectX::XMVector3TransformCoord(
			DirectX::XMVectorSet(pAppState->DefaultFwd.x, pAppState->DefaultFwd.y, pAppState->DefaultFwd.z, 0.0f),
			g_CamRotation);

		DirectX::XMVECTOR camRight = DirectX::XMVector3TransformCoord(
			DirectX::XMVectorSet(pAppState->DefaultRight.x, pAppState->DefaultRight.y, pAppState->DefaultRight.z, 0.0f),
			g_CamRotation);

		DirectX::XMVECTOR camUp = DirectX::XMVector3TransformCoord(
			DirectX::XMVectorSet(pAppState->DefaultUp.x, pAppState->DefaultUp.y, pAppState->DefaultUp.z, 0.0f),
			g_CamRotation);

		// Current camera position vector
		DirectX::XMVECTOR camPosition = DirectX::XMVectorSet(
			pAppState->pSceneState->Cam.CameraPosition.x,
			pAppState->pSceneState->Cam.CameraPosition.y,
			pAppState->pSceneState->Cam.CameraPosition.z,
			0.0f);

		// Move camera along rotated axes
		camPosition = DirectX::XMVectorAdd(camPosition, DirectX::XMVectorScale(camRight, g_MoveLeftRight));
		camPosition = DirectX::XMVectorAdd(camPosition, DirectX::XMVectorScale(camForward, g_MoveBackForward));
		camPosition = DirectX::XMVectorAdd(camPosition, DirectX::XMVectorScale(camUp, g_MoveDownUp));

		// Calculate final target position
		DirectX::XMVECTOR camTarget = DirectX::XMVectorAdd(camPosition, camForward);

		// Store back into floats
		DirectX::XMFLOAT3 cpos, cfwd, cup;
		DirectX::XMStoreFloat3(&cpos, camPosition);
		DirectX::XMStoreFloat3(&cfwd, camTarget);
		DirectX::XMStoreFloat3(&cup, camUp);

		pAppState->pSceneState->Cam.CameraPosition = float3{ cpos.x, cpos.y, cpos.z };
		pAppState->pSceneState->Cam.CameraForward = float3{ cfwd.x, cfwd.y, cfwd.z };
		pAppState->pSceneState->Cam.CameraUp = float3{ cup.x, cup.y, cup.z };
		//pAppState->pSceneState->Cam.CameraForward = Normalize(pAppState->pSceneState->Cam.CameraForward);
#else
		g_CamRotation = DirectX::XMMatrixRotationRollPitchYaw(pAppState->pSceneState->Cam.Elevation,
			pAppState->pSceneState->Cam.Azimuth, 0);
		
		DirectX::XMVECTOR camTarget = DirectX::XMVector3TransformCoord
		(DirectX::XMVectorSet(pAppState->DefaultRight.x, pAppState->DefaultRight.y, pAppState->DefaultRight.z,1.0f), 
			g_CamRotation);
		
		camTarget = DirectX::XMVector3Normalize(camTarget);

		//DirectX::XMMATRIX RotateYTempMatrix;
		//RotateYTempMatrix = DirectX::XMMatrixRotationY(pAppState->pSceneState->Cam.Azimuth);

		DirectX::XMVECTOR camRight = XMVector3TransformCoord(
			DirectX::XMVectorSet(pAppState->DefaultRight.x, pAppState->DefaultRight.y, pAppState->DefaultRight.z, 1.0f), 
			g_CamRotation);
		
		DirectX::XMVECTOR camUp = XMVector3TransformCoord(
			DirectX::XMVectorSet(pAppState->DefaultUp.x, pAppState->DefaultUp.y, pAppState->DefaultUp.z,1.0f)
			, g_CamRotation);
		
		camUp = DirectX::XMVector3Normalize(camUp);

		DirectX::XMVECTOR camForward = XMVector3TransformCoord(
			DirectX::XMVectorSet(pAppState->DefaultFwd.x, pAppState->DefaultFwd.y, pAppState->DefaultFwd.z, 1.0f)
			, g_CamRotation);

		DirectX::XMVECTOR camPosition = DirectX::XMVectorSet(pAppState->pSceneState->Cam.CameraPosition.x, pAppState->pSceneState->Cam.CameraPosition.y, pAppState->pSceneState->Cam.CameraPosition.z,0.0f);
		camPosition = DirectX::XMVectorAdd(camPosition, DirectX::XMVectorScale(camRight, g_MoveLeftRight));
		camPosition = DirectX::XMVectorAdd(camPosition, DirectX::XMVectorScale(camForward, g_MoveBackForward));
		camPosition = DirectX::XMVectorAdd(camPosition, DirectX::XMVectorScale(camUp, g_MoveDownUp));

		DirectX::XMFLOAT3 cpos;
		DirectX::XMFLOAT3 cfwd;
		DirectX::XMFLOAT3 cup;

		camTarget = DirectX::XMVectorAdd(camPosition,camTarget);

		DirectX::XMStoreFloat3(&cpos, camPosition);
		DirectX::XMStoreFloat3(&cfwd, camTarget);
		DirectX::XMStoreFloat3(&cup, camUp);

		pAppState->pSceneState->Cam.CameraPosition = float3{cpos.x,cpos.y,cpos.z};
		pAppState->pSceneState->Cam.CameraForward = float3{ cfwd.x, cfwd.y, cfwd.z };
		pAppState->pSceneState->Cam.CameraUp = float3{ cup.x,cup.y,cup.z };
#endif
		//pAppState->pSceneState->Cam.CameraForward = pAppState->pSceneState->Cam.CameraForward + pAppState->pSceneState->Cam.CameraPosition;

		g_MoveBackForward = 0.0f;
		g_MoveLeftRight = 0.0f;
		g_MoveDownUp = 0.0f;
	}

	U0 UpdateState(FAppState* pAppState)
	{
		UpdateCamera(pAppState);
	}
}