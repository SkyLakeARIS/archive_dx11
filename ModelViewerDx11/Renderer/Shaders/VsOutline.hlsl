/*
    b0 ~ b5 는 렌더러에서 예약 (reserved slots by renderer)

    b0 : World Matrix
    b1 : View + Projection Matrix (mainly Perspective)
    b2 : View + Projection Matrix of Light
    b3 : Light Attributes (for lighting)
    b4 : Camera Attribute (for lighting)
    b5 : Orthographic Matrix (for screen)
 */

cbuffer CbMatWorld : register(b0)
{
    matrix MatWorld;
}

cbuffer CbMatViewProj : register(b1)
{
    matrix MatViewProj;
}

cbuffer CbOutline : register(b6)
{
    float OutlineWidth;
}

struct VsInput
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
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
    output.Position = mul(float4(input.Normal.xyz * outlineWidth + input.Position.xyz, 1), MatWorld);
    output.Position = mul(float4(output.Position.xyz, 1), MatViewProj);
    output.TexCoord = input.TexCoord;
	return output;
}
