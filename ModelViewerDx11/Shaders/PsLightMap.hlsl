
Texture2D texDiffuse : register(t0);
Texture2D texLightMap : register(t1);

SamplerState samLinear : register(s0);


cbuffer cbMaterial : register(b0)
{
    float3 Diffuse;
    float Reserve0;
    float3 Ambient;
    float Reserve1;
    float3 Specular;
    float Reserve2;
    float3 Emissive;
    float Reserve3;
    float Opacity; // ���İ����� ���
    float Reflectivity;
    float Shininess; // ����ŧ�� �ŵ����� ��
    float Reserve4;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3 LightColor : TEXCOORD1;
    float3 LightDir : TEXCOORD2;
    float3 CameraDir : TEXCOORD3;
};


float4 main(PS_INPUT input) : SV_TARGET
{

    float4 albedo = texDiffuse.Sample(samLinear, input.UV);
    float3 normal = texLightMap.Sample(samLinear, input.UV);
    normal = normalize(normal);

    // TODO ����Ʈ���� ��� �����ؾ� �ұ�?
    float3 diffuse = dot(normalize(normal), -normalize(input.LightDir));
    diffuse = saturate(diffuse) * input.LightColor.xyz;

    float4 finalColor = (float4)0;
    finalColor.xyz = albedo;
    finalColor.a = Opacity;

	return finalColor;
}