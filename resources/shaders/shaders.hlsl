struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer ConstBuffer : register(b0)
{
    float4x4 mvp;
}

VOut VShader(float4 position : POSITION, float4 color : COLOR, float3 normal : NORMAL)
{
    VOut output;

    output.position = mul(position, mvp);
    output.color = color;

    return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}