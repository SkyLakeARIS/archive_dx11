
Texture2D txDiffuse : register(t0);

SamplerState samLinear : register(s0);

cbuffer cbMaterialFactors : register(b0)
{
    float3 Diffuse;
    float Opacity;
    float3 Ambient;
    float Reflectivity;
    float3 Specular;
    float Shininess;
    float3 Emissive;
    float IsLitOn;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 WorldPosition : TEXCOORD2;
};

struct PsOutput
{
    float4 Color : SV_TARGET0;      // w - reserved
    float4 Normal : SV_TARGET1;     // w - reserved
    float4 Position : SV_TARGET2;    // w - Lit On/Off
    float4 Specular : SV_TARGET3;   // w - Shininess
    float4 Ambient : SV_TARGET4;     // w - reserved
};

PsOutput main(PS_INPUT input)
{
    PsOutput psOutput = (PsOutput)0;
    psOutput.Color = txDiffuse.Sample(samLinear, input.Tex);
    const float Reserved = 0.0;
    psOutput.Normal = float4(0.0, 0.0, 0.0, Reserved);
    psOutput.Position = float4(input.WorldPosition, IsLitOn);
    psOutput.Specular = float4(0.0, 0.0, 0.0, 0.0);
    psOutput.Ambient = float4(0.0, 0.0, 0.0, Reserved);
    return psOutput;
}
