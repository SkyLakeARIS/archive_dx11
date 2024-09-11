struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};


float4 main(VS_INPUT input ) : SV_POSITION
{
	return input.Pos;
}