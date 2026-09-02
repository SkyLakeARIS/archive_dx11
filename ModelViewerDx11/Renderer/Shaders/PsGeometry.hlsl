Texture2D texModel : register(t0);
Texture2D texNormal : register(t1);

SamplerState sam : register(s0);


struct PsInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

struct PsOutput
{
    float3 Color : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float Depth : SV_TARGET2;
};

PsOutput main(PsInput input)
{
    PsOutput psOutput = (PsOutput)0;

    psOutput.Color = texModel.Sample(sam, input.UV).rgb;
    psOutput.Normal = texNormal.Sample(sam, input.UV);
    psOutput.Depth = input.Pos.z;
    return psOutput;
}
