//--------------------------------------------------------------------------------------
// Pixel Shader
// 여러개 만들어도 된다. 컴파일 할 때 함수명만 잘 지정해두면. (여러 셰이더 컴파일 해두고, blob만 바꿔서 런타임에 쓰도록 하는것?)
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
    float Opacity; // 알파값으로 사용
    float Reflectivity;
    float Shininess; // 스페큘러 거듭제곱 값
    float Reserve4;
}

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

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 finalColor = texModel.Sample(samLinear, input.UV);
    float3 normal = texNormal.Sample(samLinear, input.UV); // face는 _N 텍스쳐가 없음.

        // TODO ligtmap 테스트 - 별도 셰이더를 제작하는 방법도 고려
    if (Reserve4 <= 1.0f-0.001f) 
    {
        normal.z = sqrt(1.0 - (normal.x * normal.x + normal.y * normal.y));

        // 1. VS에서 계산한 노말값 사용할 때
        float3 diffuse = dot(normalize(normal), -normalize(input.LightDir));
        // 2. 텍스쳐가 노말맵이라고 가정하고 사용할 때
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
       // finalColor.xyz *= (specular + diffuse + ambient + Emissive); // 빛나는 효과 == Emissive. 켜고 끌 수 있는 기능 추가 필요
        finalColor.xyz *= (specular + diffuse + ambient);
        finalColor.a = Opacity;
    }
    else // normal 텍스쳐가 없는 face mesh
    {

        float3 diffuse = dot(normal, -input.LightDir);
        diffuse = saturate(diffuse) * Diffuse * input.LightColor.xyz * Reflectivity;

        finalColor.xyz *= (diffuse + Specular + Ambient + Emissive);
        finalColor.a = Opacity;
    }

    //float3 lightmapColor = 0.0f;
    //if (Reserve4 == 1.0f)
    //{
    //    lightmapColor = texNormal.Sample(samLinear, input.UV); // face는 _N 텍스쳐가 없음.
    //}

    // 보통 픽셀 단위로 조정하고 싶을 때 텍스쳐로 제작한다면
    // normal 값도 마찬가지로.. 그럼 VS에서 쓴 normal과 다른데..
    //finalColor += saturate(dot(input.Norm, (float3)vLightDir) * vLightColor);
 

    return finalColor;
}