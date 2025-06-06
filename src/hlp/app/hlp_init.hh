#pragma once
#include "../hlp_state.hh"

namespace hlp
{
#define HLP_DEFAULT_CAM_POS float3{0.f, 0.0f, -4.0f}
#define HLP_DEFAULT_CAM_FWD float3{0.f,0.f,0.f}
#define HLP_DEFAULT_CAM_UP float3{0.,1.,0.}
#define HLP_DEFAULT_ASPECT_RATIO (16.f / 9.f)
#define HLP_DEFAULT_HFOV 90.f
#define HLP_DEFAULT_NEAR_PLANE 1.0f
#define HLP_DEFAULT_FAR_PLANE 1000.f
#define HLP_DEFAULT_AZIMUTH 0.0f
#define HLP_DEFAULT_ELEVATION 0.0f 

#define HLP_DEFAULT_CAM_SPEED 0.001f
#define HLP_DEFAULT_MOUSE_SENS 0.01f

#define HLP_DEFAULT_DEFAULT_FWD float3{0.0f,0.0f,1.0f}
#define	HLP_DEFAULT_DEFAULT_RIGHT float3{1.0f, 0.0f, 0.0f}
#define	HLP_DEFAULT_DEFAULT_UP float3{0.0f,1.0f,0.0f}
#define	HLP_DEFAULT_DTIME 1.0f

#define HLP_DEFAULT_WIDTH 1920
#define HLP_DEFAULT_HEIGHT 1080
#define HLP_DEFAULT_SHOW_WND 1
#define HLP_DEFAULT_FPS 60
}