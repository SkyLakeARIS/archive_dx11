
cbuffer cbMatView : register(b0)
{
    matrix MatWorld;
}

cbuffer cbMatOrtho : register(b1)
{
    matrix MatOrtho;
}


struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};


struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};



PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    output.Pos = mul(input.Pos, MatWorld);
    output.Pos = mul(output.Pos, MatOrtho);

    output.Tex = input.Tex;
    return output;
}