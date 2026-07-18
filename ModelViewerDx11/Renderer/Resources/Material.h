#pragma once
#include "RenderTypes.h"
#include "TextureData.h"

namespace renderer
{
    // 모델링 프로그램에서 미리 계산된 값으로 사용
    // MEMO: Shader에 바로 넘길 수 있도록 별도로 데이터 구조 분리.
    struct MaterialParameter // 16 bytes align
    {
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

    // MEMO: Material을 질감 데이터+셰이더+텍스처+렌더 상태의 집합으로 구조를 잡음.
    // 렌더 상태까지 한곳에 있어 상태 관리가 편해질 것
    inline constexpr int8_t CONSTANT_BUFFER_MAX_SLOT_COUNT = 8;
    struct Material
    {
        // MEMO: 재질
        // TODO: improve - 좀 더 깔끔한 네이밍이 있을지.?
        MaterialParameter MaterialParam;
        // MEMO: 텍스처
        HashID TextureHashes[static_cast<uint8_t>(eTextureType::TextureTypeCount)];
        // MEMO: 셰이더
        // TODO: Shader도 각 Shader마다 CB 슬롯과 상태별 Bind Slot들을 매핑해줄 무언가가 필요함.
        eShader ShaderType;
        uint32_t ConstantBuffers[CONSTANT_BUFFER_MAX_SLOT_COUNT];
        // MEMO: 렌더 상태
        eRasterType RasterType;
        eSamplerType SamplerType;
        HashID BlendHash;
        ePrimitiveTopology TopologyType;
        // TODO: improve - 현재 옵션이 Skybox 전용으로만 존재하므로 확장이 필요함.
        bool bUseDepthStencil;
    };

}
