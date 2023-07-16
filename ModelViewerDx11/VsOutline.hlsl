cbuffer cbOutline : register(b0)
{
    matrix WorldViewProjectionMat;
}

struct VsInput
{
    float4 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct PsInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

PsInput main(VsInput input)
{
    PsInput output;
    float outlineWidth = 0.01f;
    output.Position = mul(float4(input.Normal.xyz * outlineWidth + input.Position.xyz, 1), WorldViewProjectionMat);
    output.TexCoord = input.TexCoord;
	return output;
}