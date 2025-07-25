#pragma once 
#include "hlp_declarations.hh"
#include <stdio.h>
// settings

#define HLP_D3D12 1
#define HLP_D3D12_PS_LOC L"..\\resources\\shaders\\PixelShader.hlsl"
#define HLP_D3D12_VS_LOC L"..\\resources\\shaders\\VertexShader.hlsl"


#define HLP_SHADER_LOCATION L"..\\resources\\shaders\\shaders.hlsl"
#define HLP_TEXTURE_LOCATION L"..\\resources\\textures\\tex.jpg"
#define HLP_MOUSE_SENS 0.001f

#define HLP_HARDCODE_MVP 1
#define HLP_DB_WIN32_FILE 1

// macros
#if defined(_MSC_VER)
#define HLP_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define HLP_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define HLP_FORCE_INLINE inline
#endif

#define HLP_HANDLE_HRESULT(hr) \
    do { \
        if (FAILED(hr)) { \
            printf("FAILED: %s, %d\n", __FILE__, __LINE__); \
        } \
    } while (0)

#define HLP_ASSERT(a) (assert(a))

#define HLP_RELEASE(p) if(p){ (p)->Release(); (p) = nullptr; }