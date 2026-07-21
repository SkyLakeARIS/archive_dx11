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

cbuffer CbMatLightViewProj : register(b2)
{
    matrix MatLightViewProj;
}

struct VsInput
{
    float4 Pos : POSITION;
};

struct PsInput
{
    float4 Pos : SV_POSITION;
};

PsInput main(VsInput input)
{
    PsInput output;
    output.Pos = mul(input.Pos, MatWorld);
    output.Pos = mul(output.Pos, MatLightViewProj);
    return output;
}
