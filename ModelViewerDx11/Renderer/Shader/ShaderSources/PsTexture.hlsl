
Texture2D txDiffuse : register(t0);

SamplerState texSampler : register(s0);

struct PsInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PsInput psInput) : SV_TARGET
{
    return txDiffuse.Sample(texSampler, psInput.TexCoord);
}
