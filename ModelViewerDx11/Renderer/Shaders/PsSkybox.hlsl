
TextureCube cubeTexture : register(t0);

SamplerState cubeSampler : register(s0);

struct PsInput
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD0;
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
    psOutput.Color = cubeTexture.Sample(cubeSampler, input.TexCoord);
    psOutput.Normal = float3(1.0, 0.0, 0.0);
    psOutput.Depth = input.Position.z;
    return psOutput;
}
