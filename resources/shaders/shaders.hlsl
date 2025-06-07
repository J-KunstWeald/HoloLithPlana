cbuffer cbPerObject
{
	float4x4 WVP;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

VS_OUTPUT VShader(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD)
{
    VS_OUTPUT output;

    output.Pos = mul(inPos, WVP);
    output.TexCoord = inTexCoord;

    return output;
}

float4 PShader(VS_OUTPUT input) : SV_TARGET
{
    return ObjTexture.Sample( ObjSamplerState, input.TexCoord );
}

//struct VOut
//{
//    float4 position : SV_POSITION;
//    float2 texCoord : COLOR;
//};

//cbuffer ConstBuffer : register(b0)
//{
//    float4x4 mvp;
//}

//VOut VShader(float4 position : POSITION, float2 color : COLOR, float3 normal : NORMAL)
//{
//    VOut output;
//
//    output.position = mul(position, mvp);
//    output.color = color;
//
//    return output;
//}

//float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
//{
//    return color;
//}