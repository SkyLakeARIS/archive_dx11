#pragma once
#include "TextureData.h"

namespace renderer
{
    // 모델링 프로그램에서 미리 계산된 값으로 사용
    // MEMO: Shader에 바로 넘길 수 있도록 별도로 데이터 구조 분리.
    struct MaterialParameter // 16 bytes align
    {
        // TODO: optimize - packing을 좀 더 타이트하게: Reserve slot에 float값들로 교체하기 
        XMFLOAT3 Diffuse;
        float    Reserve0;
        XMFLOAT3 Ambient;
        float    Reserve1;
        XMFLOAT3 Specular;
        float    Reserve2;
        XMFLOAT3 Emissive;
        float    Reserve3;
        float    Opacity;       // 알파값으로 사용
        float    Reflectivity;
        float    Shininess;     // 스페큘러 거듭제곱 값
        float    Reserve4;
    };
    typedef MaterialParameter CbMaterial;

    // MEMO:(신버전) Material에서 셰이더 + 렌더 상태를 분리(struct RenderState)하고 SemiMaterial로 변경.
    // 현재 구조로는 material이 여러 셰이더(패스)를 제공하지 못하고, RenderPacket이 정보를 가지면 Material 시스템의 의미가 퇴색되므로 구조를 축소함.
    // (구버전)Material을 질감 데이터+셰이더+텍스처+렌더 상태의 집합으로 구조를 잡음.
    struct SemiMaterial
    {
        // MEMO: 재질
        // TODO: improve - 좀 더 깔끔한 네이밍이 있을지.?
        MaterialParameter MaterialParam;
        XMFLOAT3 OutlineWidth;
        // MEMO: 텍스처
        HashID TextureHashes[static_cast<uint8_t>(eTextureType::TextureTypeCount)];
    };

}
