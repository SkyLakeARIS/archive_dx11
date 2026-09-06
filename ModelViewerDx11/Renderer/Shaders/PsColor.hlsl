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

struct PsInput
{
    float4 Pos : SV_POSITION;
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

PsOutput main(PsInput input)
{
    PsOutput psOutput = (PsOutput)0;
    const float Reserved = 0.0;
    psOutput.Color = float4(Diffuse, Reserved);
    psOutput.Normal = float4(0.0, 0.0, 0.0, Reserved);
    psOutput.Position = float4(input.WorldPosition, IsLitOn);
    psOutput.Specular = float4(0.0, 0.0, 0.0, 0.0);
    psOutput.Ambient = float4(0.0, 0.0, 0.0, Reserved);
    return psOutput;
}
