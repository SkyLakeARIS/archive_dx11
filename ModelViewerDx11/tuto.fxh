//--------------------------------------------------------------------------------------
// File: Tutorial02.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------

cbuffer cbNerverChanges : register(b0)
{
    matrix View;
}

cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
}

cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
}

cbuffer cbLight : register(b3)
{
    float4 vLightColor;
    float4 vLightDir;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Norm = mul(float4(input.Norm, 1), World).xyz;

    return output;
}


float4 PS_Lighting( PS_INPUT input) : SV_Target
{
    float4 finalColor = (float4)0;
    finalColor += saturate(dot(input.Norm, (float3)vLightDir) * vLightColor)-0.3f;
    finalColor.a = 1;
    return finalColor;
}
