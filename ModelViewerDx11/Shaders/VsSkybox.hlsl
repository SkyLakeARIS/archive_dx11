
cbuffer CbWVP : register(b0)
{
    matrix MatWorld;
}

cbuffer CbVP : register(b1)
{
    matrix MatViewProj;
}

struct VsInput
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct PsInput
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD0;
};

PsInput main(VsInput input)
{
    PsInput output;

    // ���� ���� ���� �ھ�� �ϱ� ������ ���� �ٽ� �����ϱ� ���� 1�� w���� ��� ���� ��. w = 1, using w instead of z
    //output.Position = mul(float4(input.Position.xyz, 1), MatWorld).xyww;
    output.Position = mul(input.Position, MatWorld);
    output.Position = mul(float4(output.Position.xyz, 1), MatViewProj).xyww;

    // tex���� ��ġ ��
    output.TexCoord = input.Position;
    return output;
}