#include "hlp_defines.hh"
#include "hlp_math.hh"


namespace hlp
{
	float4x4 MatMul(float4x4 const* pA, float4x4 const* pB)
	{
		float4x4 const& A = *pA;
		float4x4 const& B = *pB;

		// C = A * B
		float4x4 C = { };
		for (I32 i = 0; i < 4; i++)
		{
			for (I32 j = 0; j < 4; j++)
			{
				for (I32 k = 0; k < 4; k++)
				{
					C[j][i] += A[k][i] * B[j][k];
				}
			}
		}
		return C;
	}
}