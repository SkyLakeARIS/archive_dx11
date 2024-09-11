
cbuffer cbMatrices : register(b0)
{
    matrix World;
    matrix WVP;
}

cbuffer cbLightMatrix : register(b1)
{
    matrix WVPLight;
}

cbuffer cbLight : register(b2)
{
    float4 vLightColor;
    float4 vLightDir;
}

cbuffer CbCamera : register(b3)
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
    float2 UV : TEXCOORD0;
    float3 LightColor : TEXCOORD1;
    float3 LightDir : TEXCOORD2;
    float3 CameraDir : TEXCOORD3;
    float4 ClipPosition : TEXCOORD4;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;

    float3 worldPosition = mul(input.Pos, World).xyz;

    output.Pos = mul(input.Pos, WVP);
    output.ClipPosition = mul(input.Pos, WVPLight);

    output.LightDir = normalize(worldPosition - vLightDir);

    output.UV = input.Tex;
    output.LightColor = vLightColor.xyz;
    output.CameraDir = normalize(worldPosition - Position);

    return output;
}
