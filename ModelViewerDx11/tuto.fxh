//--------------------------------------------------------------------------------------
// File: Tutorial02.fx
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License (MIT).
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

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
    float4 vMeshColor;
}

cbuffer cbLight : register(b3)
{
    float4 vLightColor;
    float4 vLightDir;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
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
    output.Tex = input.Tex;
    output.Norm = mul(float4(input.Norm, 1), World).xyz;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
// 여러개 만들어도 된다. 컴파일 할 때 함수명만 잘 지정해두면. (여러 셰이더 컴파일 해두고, blob만 바꿔서 런타임에 쓰도록 하는것?)
//--------------------------------------------------------------------------------------
float4 PS_TextureAndLighting( PS_INPUT input) : SV_Target
{
    float4 finalColor = txDiffuse.Sample(samLinear, input.Tex);
    finalColor += saturate(dot(input.Norm, (float3)vLightDir) * vLightColor);
    finalColor.a = 1;
    return finalColor;
}

float4 PS_Texture( PS_INPUT input) : SV_Target
{
    return txDiffuse.Sample(samLinear, input.Tex)*vMeshColor;
}
