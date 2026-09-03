cbuffer cbColor : register(b0)
{
    float3 Color;
    float Reserve;
}


struct PsInput
{
    float4 Pos : SV_POSITION;
};

struct PsOutput
{
    float3 Color : SV_TARGET0;
    float3 Normal : SV_TARGET1;
    float Depth : SV_TARGET2;
};

PsOutput main(PsInput input)
{
    PsOutput psOutput = (PsOutput)0;
    psOutput.Color = Color;
    psOutput.Normal = float3(1.0, 0.0, 0.0);
    psOutput.Depth = input.Pos.z;
    return psOutput;
}
