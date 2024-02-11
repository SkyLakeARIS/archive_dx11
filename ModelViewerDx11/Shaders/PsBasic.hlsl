//--------------------------------------------------------------------------------------
// Pixel Shader
// ������ ���� �ȴ�. ������ �� �� �Լ��� �� �����صθ�. (���� ���̴� ������ �صΰ�, blob�� �ٲ㼭 ��Ÿ�ӿ� ������ �ϴ°�?)
//--------------------------------------------------------------------------------------
Texture2D texModel : register(t0);
Texture2D texNormal : register(t1);

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
    float4 finalColor = texModel.Sample(samLinear, input.UV);
    float3 normal = texNormal.Sample(samLinear, input.UV); // face�� _N �ؽ��İ� ����.

    normal.z = sqrt(1.0 - (normal.x * normal.x + normal.y * normal.y));

    // 1. VS���� ����� �븻�� ����� ��
    float3 diffuse = dot(normalize(normal), -normalize(input.LightDir));
    // 2. �ؽ��İ� �븻���̶�� �����ϰ� ����� ��
    //float3 diffuse = dot(normal, -input.LightDir);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    if (diffuse.x > 0)
    {
        float3 reflection = normalize(reflect(input.LightDir, normalize(normal)));
        specular = saturate(dot(reflection, normalize(-input.CameraDir)));
        specular = pow(specular, Shininess) * Reflectivity;
        specular *= Specular * input.LightColor.xyz;
    }

    diffuse = saturate(diffuse) * input.LightColor.xyz * Reflectivity;

    float3 ambient = Ambient * input.LightColor;
   // finalColor.xyz *= (specular + diffuse + ambient + Emissive); // ������ ȿ�� == Emissive. �Ѱ� �� �� �ִ� ��� �߰� �ʿ�
    finalColor.xyz *= (specular + diffuse + ambient);
    finalColor.a = Opacity;

    return finalColor;
}