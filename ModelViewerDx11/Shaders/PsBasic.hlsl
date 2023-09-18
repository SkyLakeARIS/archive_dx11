//--------------------------------------------------------------------------------------
// Pixel Shader
// 여러개 만들어도 된다. 컴파일 할 때 함수명만 잘 지정해두면. (여러 셰이더 컴파일 해두고, blob만 바꿔서 런타임에 쓰도록 하는것?)
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