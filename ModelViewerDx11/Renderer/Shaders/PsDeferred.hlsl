Texture2D texGColor : register(t0);
Texture2D texGNormal: register(t1);
Texture2D texGDepth: register(t2);

SamplerState sam : register(s0);


struct PsInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

float4 main(PsInput input) : SV_TARGET
{
    float4 outColor = 0;

    outColor = texGColor.Sample(sam, input.UV);
    return outColor;
}
