cbuffer CbLightMatrix : register(b0)
{
    matrix MVP;
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
    output.Pos = mul(input.Pos, MVP);
    return output;
}