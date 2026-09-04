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

cbuffer CbMatLightViewProj : register(b2)
{
    matrix MatLightViewProj;
}

cbuffer CbLightProperty : register(b3)
{
    float4 vLightColor;
    float4 vLightDir;
}

cbuffer CbCamera : register(b4)
{
    float3 Position;
    float Reserve;
}

struct VsInput
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PsInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

PsInput main(VsInput vsInput)
{
    PsInput psInput = (PsInput)0;
    psInput.Pos = vsInput.Pos;
    psInput.UV = vsInput.Tex;
	return psInput;
}
