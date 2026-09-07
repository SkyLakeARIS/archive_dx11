Texture2D texGColor : register(t0);
Texture2D texGNormal: register(t1);
Texture2D texGPosition: register(t2);
Texture2D texGSpecular: register(t3);
Texture2D texGAmbient: register(t4);
Texture2D texShadow: register(t5);

SamplerState sam : register(s0);


struct PsInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    nointerpolation float3 CameraPosition : TEXCOORD1;
    nointerpolation float4 LightColor : TEXCOORD2;
    nointerpolation float4 LightDir : TEXCOORD3;
    nointerpolation matrix MatLightViewProj : TEXCOORD4;
};

float4 main(PsInput input) : SV_TARGET
{
    float3 outColor = texGColor.Sample(sam, input.UV).rgb;
    const float3 worldNormal = normalize(texGNormal.Sample(sam, input.UV).xyz);
    const float4 worldPosition = texGPosition.Sample(sam, input.UV);
    const float IsLitOn = worldPosition.w;
    const float4 specular = texGSpecular.Sample(sam, input.UV);
    const float3 ambient = texGAmbient.Sample(sam, input.UV).rgb;

    // MEMO: Lit On인 픽셀만
    if (IsLitOn > 0.0)
    {
        float4 lightClipSpace = mul(float4(worldPosition.xyz, 1.0), input.MatLightViewProj);
        lightClipSpace = lightClipSpace / lightClipSpace.w;
        const float2 shadowTexCoord = float2(lightClipSpace.xy) * float2(0.5, -0.5) + 0.5;
        const float depthInShadowMap = texShadow.Sample(sam, shadowTexCoord).r;

        const float DEPTH_OFFSET = 0.00001;
        if (lightClipSpace.z > depthInShadowMap + DEPTH_OFFSET)
        {
            // MEMO: ambient 색상을 그림자 색상으로 사용하는 것이 시도한 것 중 가장 자연스러움
            outColor *= ambient * input.LightColor.rgb;
        }
        else
        {
            const float diffuseIntensity = saturate(dot(normalize(-input.LightDir.xyz), worldNormal));
            float specularIntensity = 0.0;
            if (diffuseIntensity > 0.0)
            {
                const float3 reflectLight = normalize(reflect(input.LightDir.xyz, worldNormal));
                // MEMO: 미리 뒤집어서 사용
                const float3 cameraDir = normalize(input.CameraPosition - worldPosition.xyz);
                specularIntensity = dot(reflectLight, cameraDir);
                specularIntensity = pow(saturate(specularIntensity), specular.w);
            }
            outColor *= (diffuseIntensity + specular.xyz * specularIntensity + ambient) * input.LightColor.rgb;
        }
    }
    return float4(outColor, 1.0);
}
