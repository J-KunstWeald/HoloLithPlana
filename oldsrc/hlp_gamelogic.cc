#include "hlp_gamelogic.hh"

namespace hlp
{

	U0 UpdateGameLogicState(FGameLogicState* pGLS, FInputState* pIS) 
	{

	}

	U0 UpdateCamera(FCamera* cam, FInputState* input, F32 camspeed, F32 deltatime)
	{
		// front/back
		if (input->bMoveForward == input->bMoveBackward)
		{
			// counterstep noop
		}
		else
		{
			if (input->bMoveForward)
			{
				cam->CameraPosition = cam->CameraPosition + camspeed * deltatime * cam->CameraForward;
			}
			else if (input->bMoveBackward)
			{
				cam->CameraPosition = cam->CameraPosition - camspeed * deltatime * cam->CameraForward;
			}
		}

		if (input->bMoveLeft == input->bMoveRight)
		{
			// counterstep noop
		}
		else
		{
			if (input->bMoveLeft)
			{
				cam->CameraPosition = cam->CameraPosition + camspeed * deltatime * Normalize(Cross(cam->CameraUp, cam->CameraForward)); // cam->CameraForward,cam->CameraUp
			}
			else if (input->bMoveRight)
			{
				cam->CameraPosition = cam->CameraPosition - camspeed * deltatime * Normalize(Cross(cam->CameraUp, cam->CameraForward));
			}
		}

		if (input->bMoveUp == input->bMoveDown)
		{
			// counterstep noop
		}
		else
		{
			if (input->bMoveUp)
			{
				cam->CameraPosition = cam->CameraPosition + camspeed * deltatime * cam->CameraUp;
			}
			else if (input->bMoveDown)
			{
				cam->CameraPosition = cam->CameraPosition - camspeed * deltatime * cam->CameraUp;
			}
		}

		if (input->LastMouseX == input->MouseX && input->LastMouseY == input->MouseY)
		{
			// no move noop
		}
		else
		{
#if 1
			// deltaX = MouseX - LastMouseX
			// deltaY = LastMouseY - MouseY
			// azimuth += deltaX * sens 
			// elevation += deltaY * sens 

			float3 RightDir = Cross(cam->CameraUp, cam->CameraForward);

			cam->Azimuth += HLP_MOUSE_SENS * (input->LastMouseX - input->MouseX);
			cam->Elevation += HLP_MOUSE_SENS * (input->MouseY - input->LastMouseY);
			input->LastMouseX = input->MouseX;
			input->LastMouseY = input->MouseX;

			if (cam->Elevation > 89.f)
				cam->Elevation = 89.f;
			if (cam->Elevation < -89.f)
				cam->Elevation = -89.f;

			cam->CameraForward = Normalize(float3{
				cosf(HLP_TO_RADIAN(cam->Azimuth)) * cosf(HLP_TO_RADIAN(cam->Elevation)),
				sinf(HLP_TO_RADIAN(cam->Elevation)),
				sinf(HLP_TO_RADIAN(cam->Azimuth)) * cosf(HLP_TO_RADIAN(cam->Elevation))
				});

			cam->CameraUp = Cross(cam->CameraForward, RightDir);
#else
			// deltaX = MouseX - LastMouseX
			// deltaY = LastMouseY - MouseY
			// azimuth += deltaX * sens 
			// elevation += deltaY * sens 

			float3 RightDir = Cross(cam->CameraUp, cam->CameraForward);

			F32 Azimuth = HLP_MOUSE_SENS * (input->MouseX - input->LastMouseX);
			F32 Elevation = HLP_MOUSE_SENS * (input->LastMouseY - input->MouseX);
			input->LastMouseX = input->MouseX;
			input->LastMouseY = input->MouseX;

			if (Elevation > 89.f)
				Elevation = 89.f;
			if (Elevation < -89.f)
				Elevation = -89.f;

			cam->CameraForward = Normalize(float3{
				cosf(HLP_TO_RADIAN(Azimuth)) * cosf(HLP_TO_RADIAN(Elevation)),
				sinf(HLP_TO_RADIAN(Elevation)),
				sinf(HLP_TO_RADIAN(Azimuth)) * cosf(HLP_TO_RADIAN(Elevation))
				});

			cam->CameraUp = Cross(cam->CameraForward, RightDir);
#endif
		}
	}

}