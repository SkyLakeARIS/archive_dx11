//--------------------------------------------------------------------------------------
// Pixel Shader
// 여러개 만들어도 된다. 컴파일 할 때 함수명만 잘 지정해두면. (여러 셰이더 컴파일 해두고, blob만 바꿔서 런타임에 쓰도록 하는것?)
//--------------------------------------------------------------------------------------
Texture2D texModel : register(t0);
Texture2D texNormal : register(t1);
Texture2D texShadow : register(t2);

SamplerState samLinear : register(s0);


cbuffer cbMaterialFactors : register(b0)
{
    float3 Diffuse;
    float Opacity;   // 알파값으로 사용
    float3 Ambient;
    float Reflectivity;
    float3 Specular;  // 스페큘러 거듭제곱 값
    float Shininess;
    float3 Emissive;
    float Reserve1;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

struct PsOutput
{
    float3 Color : SV_TARGET0;
    float3 Normal : SV_TARGET1;
    float Depth : SV_TARGET2;
};


PsOutput main(PS_INPUT input)
{
    PsOutput psOutput = (PsOutput)0;

    psOutput.Color = texModel.Sample(samLinear, input.UV);
    psOutput.Normal = texNormal.Sample(samLinear, input.UV); // face는 _N 텍스쳐가 없음.
    psOutput.Depth = input.Pos.z;

    return psOutput;
}
