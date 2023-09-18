//--------------------------------------------------------------------------------------
// Pixel Shader
// ������ ���� �ȴ�. ������ �� �� �Լ��� �� �����صθ�. (���� ���̴� ������ �صΰ�, blob�� �ٲ㼭 ��Ÿ�ӿ� ������ �ϴ°�?)
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);

SamplerState samLinear : register(s0);

cbuffer cbLight : register(b0)
{
    float4 vLightColor;
    float4 vLightDir;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 finalColor = txDiffuse.Sample(samLinear, input.Tex);
    finalColor += saturate(dot(input.Norm, (float3)vLightDir) * vLightColor);
    finalColor.a = 1;
    return finalColor;
}