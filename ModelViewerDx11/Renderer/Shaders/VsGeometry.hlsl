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
    vsInput.Pos = mul(MatWorld, vsInput.Pos);
    psInput.Pos = mul(MatViewProj, vsInput.Pos);
    psInput.UV = vsInput.Tex;
	return psInput;
}
