#pragma once
#include <cstdint>
#include <assert.h>
namespace hlp
{
	// types

	typedef void		U0;

	typedef int8_t		I8;
	typedef int16_t		I16;
	typedef int32_t		I32;
	typedef int64_t		I64;

	typedef uint8_t		U8;
	typedef uint16_t	U16;
	typedef uint32_t	U32;
	typedef uint64_t	U64;

	typedef float		F32;
	typedef double		F64;

	struct float3
	{
		F32 x;
		F32 y;
		F32 z;
	};

	struct float4
	{
		F32 x;
		F32 y;
		F32 z;
		F32 w;

		F32& operator[](U32 i)
		{
			assert(i < 4);
			return *((&x) + i);
		}

		F32 const& operator[](U32 i) const
		{
			assert(i < 4);
			return *((&x) + i);
		}
	};


	struct float4x4
	{
		float4 columns[4];

		float4& operator[](U32 i)
		{
			assert(i < 4);
			return columns[i];
		}

		float4 const& operator[](U32 i) const
		{
			assert(i < 4);
			return columns[i];
		}
	};
}