
Texture2D txDiffuse : register(t0);

SamplerState samLinear : register(s0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

struct PsOutput
{
    float3 Color : SV_TARGET0;
    float3 Normal : SV_TARGET1;
    float Depth : SV_TARGET2;
};

PsOutput main(PS_INPUT input)
{
    PsOutput psOutput = (PsOutput)0;
    psOutput.Color = txDiffuse.Sample(samLinear, input.Tex);
    psOutput.Normal = float3(1.0, 0.0, 0.0);
    psOutput.Depth = input.Pos.z;
    return psOutput;
}
