struct VsInput
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
};


float4 main(VsInput vsInput) : SV_POSITION
{
	return vsInput.Position;
}
