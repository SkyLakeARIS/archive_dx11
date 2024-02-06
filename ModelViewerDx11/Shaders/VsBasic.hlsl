
cbuffer cbMatrices : register(b0)
{
    matrix World;
    matrix WVP;
}

cbuffer cbLight : register(b1)
{
    float4 vLightColor;
    float4 vLightDir;
}

cbuffer CbCamera : register(b2)
{
    float3 Position;
    float Reserve;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPositon : Position0;
    float2 UV : TEXCOORD0;
    float3 Norm : NORMAL;
    float3 Diffuse : TEXCOORD1;
    float3 LightColor : TEXCOORD2;
    float3 LightDir : TEXCOORD3;
    float3 CameraDir : TEXCOORD4;
   // float3 vReflection : TEXCOORD5;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;

    float3 worldPosition = mul(input.Pos, World).xyz;
    output.WorldPositon = worldPosition;

    output.Pos = mul(input.Pos, WVP);

    output.Norm = mul(float4(input.Norm, 1), World).xyz;
    output.Norm = normalize(output.Norm);

    output.LightDir = normalize(worldPosition - vLightDir);

    output.UV = input.Tex;
    output.LightColor = vLightColor.xyz;
    output.CameraDir = normalize(worldPosition - Position);

   // output.vReflection = normalize(reflect(-output.LightDir, normalize(output.Norm)));

    return output;
}
