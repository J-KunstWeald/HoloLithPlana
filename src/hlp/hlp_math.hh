#pragma once
#include <cmath>
#include <assert.h>
#include <stdint.h>
#include "hlp_defines.hh"

namespace hlp
{
	// Constants

#define HLP_MATH_PI 3.14159265359

// ======================================================================================================================================================
// float3 
// ======================================================================================================================================================


	HLP_FORCE_INLINE float3 operator*(float3 const& v, F32 s) { return float3{ v.x * s, v.y * s, v.z * s }; }
	HLP_FORCE_INLINE float3 operator*(F32 s, float3 const& v) { return float3{ v.x * s, v.y * s, v.z * s }; }
	HLP_FORCE_INLINE float3 operator/(float3 const& v, F32 s) { return float3{ v.x / s, v.y / s, v.z / s }; }
	HLP_FORCE_INLINE float3 operator/(F32 s, float3 const& v) { return float3{ s / v.x, s / v.y, s / v.z }; }

	HLP_FORCE_INLINE float3 operator+(float3 const& v, F32 s) { return float3{ v.x + s, v.y + s, v.z + s }; }
	HLP_FORCE_INLINE float3 operator+(F32 s, float3 const& v) { return float3{ v.x + s, v.y + s, v.z + s }; }

	HLP_FORCE_INLINE float3& operator*=(float3& v, F32 s) { v.x *= s; v.y *= s; v.z *= s; return v; }

	HLP_FORCE_INLINE float3& operator/=(float3& v, F32 s) { v.x /= s; v.y /= s; v.z /= s; return v; }

	HLP_FORCE_INLINE float3 operator*(float3 const& a, float3 const& b) { return float3{ a.x * b.x, a.y * b.y, a.z * b.z }; }
	HLP_FORCE_INLINE float3 operator/(float3 const& a, float3 const& b) { return float3{ a.x / b.x, a.y / b.y, a.z / b.z }; }
	HLP_FORCE_INLINE float3 operator+(float3 const& a, float3 const& b) { return float3{ a.x + b.x, a.y + b.y, a.z + b.z }; }
	HLP_FORCE_INLINE float3 operator-(float3 const& a, float3 const& b) { return float3{ a.x - b.x, a.y - b.y, a.z - b.z }; }

	HLP_FORCE_INLINE F32 Dot(float3 const& a, float3 const& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	HLP_FORCE_INLINE F32 Length(float3 const& a)
	{
		return sqrt(Dot(a, a));
	}

	HLP_FORCE_INLINE float3 Normalize(float3 const& a)
	{
		return a / Length(a);
	}

	HLP_FORCE_INLINE float3 Cross(float3 const& a, float3 const& b)
	{
		return float3{
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		};
	}

	// ======================================================================================================================================================
	// float4 
	// ======================================================================================================================================================



	HLP_FORCE_INLINE float4 operator*(float4 const& v, F32 s) { return float4{ v.x * s, v.y * s, v.z * s, v.w * s }; }
	HLP_FORCE_INLINE float4 operator*(F32 s, float4 const& v) { return float4{ v.x * s, v.y * s, v.z * s, v.w * s }; }
	HLP_FORCE_INLINE float4 operator/(float4 const& v, F32 s) { return float4{ v.x / s, v.y / s, v.z / s, v.w / s }; }
	HLP_FORCE_INLINE float4 operator/(F32 s, float4 const& v) { return float4{ s / v.x, s / v.y, s / v.z, s / v.w }; }

	HLP_FORCE_INLINE float4 operator+(float4 const& v, F32 s) { return float4{ v.x + s, v.y + s, v.z + s, v.w + s }; }
	HLP_FORCE_INLINE float4 operator+(F32 s, float4 const& v) { return float4{ v.x + s, v.y + s, v.z + s, v.w + s }; }

	HLP_FORCE_INLINE float4& operator*=(float4& v, F32 s) { v.x *= s; v.y *= s; v.z *= s; v.w *= s; return v; }

	HLP_FORCE_INLINE float4& operator/=(float4& v, F32 s) { v.x /= s; v.y /= s; v.z /= s; v.w /= s; return v; }

	HLP_FORCE_INLINE float4 operator*(float4 const& a, float4 const& b) { return float4{ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }
	HLP_FORCE_INLINE float4 operator/(float4 const& a, float4 const& b) { return float4{ a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; }
	HLP_FORCE_INLINE float4 operator+(float4 const& a, float4 const& b) { return float4{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
	HLP_FORCE_INLINE float4 operator-(float4 const& a, float4 const& b) { return float4{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }

	HLP_FORCE_INLINE F32 Dot(float4 const& a, float4 const& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	HLP_FORCE_INLINE F32 Length(float4 const& a)
	{
		return sqrt(Dot(a, a));
	}

	HLP_FORCE_INLINE float4 Normalize(float4 const& a)
	{
		return a / Length(a);
	}



	float4x4 MatMul(float4x4 const* pA, float4x4 const* pB);

	HLP_FORCE_INLINE float4x4 operator*(float4x4 A, float4x4 B) { return MatMul(&A, &B); }

	// Trigonometric
#define HLP_TO_RADIAN(deg) (deg * (HLP_MATH_PI / 180.f))
#define HLP_TO_DEGREE(rad) (rad * (180.f / HLP_MATH_PI))
}