#pragma once
#include "TextureData.h"
#include "../../framework.h"

namespace renderer
{
    enum class eSamplerType : uint8_t;
    enum class eRasterType : uint8_t;
    enum class eShader : uint8_t;
}

namespace renderer
{
    enum class ePrimitiveTopology : uint8_t
    {
        Triangles,
        TriangleStrip,
        Lines,
        TopologyCount
    };

    enum class eVertexFormat : uint8_t
    {
        PTN,    // pos, normal, tex
        PT,     // pos, tex
        P,      // pos
        FormatCount
    };

    struct VertexPTN // 4bytes align
    {
        XMFLOAT3 Position;
        XMFLOAT2 TexCoord;
        XMFLOAT3 Normal;
        float    Reserve1;
    };

    struct VertexPT // 4bytes align
    {
        XMFLOAT3 Position;
        XMFLOAT2 TexCoord;
    };

    struct VertexP // 4bytes align
    {
        XMFLOAT3 Position;
    };

    inline constexpr int16_t GetVertexStrideSize(eVertexFormat vertexAttrib)
    {
        constexpr int16_t VertexStrideMap[static_cast<int8_t>(eVertexFormat::FormatCount)] =
        {
            sizeof(VertexPTN),
            sizeof(VertexPT),
            sizeof(VertexP)
        };
        return VertexStrideMap[static_cast<int8_t>(vertexAttrib)];
    }

    // TODO: cleanup - Material은 별도 헤더 파일로 분리하기.
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
        HashID TextureHashes[eTextureType::TextureTypeCount];
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

    struct BufferRange
    {
        int32_t StartIndex;
        int32_t Count;
    };

    typedef struct CbMatrix
    {
        XMMATRIX Matrix;
    }CbWorld, CbViewProj, CbLightViewProjMatrix, CbScreenSpaceMatrix;

    typedef struct CbFloat3
    {
        XMFLOAT3    Float3;
        float       Reserve;
    } CbCameraPosition, CbOutlineProperty, CbColor;

    typedef struct CbTwoVec4
    {
        XMFLOAT4    First;
        XMFLOAT4    Second;
    }CbLightProperty;

    enum class eCbType : uint8_t
    {
        CbWorld,
        CbViewProj,
        CbLightViewProjMatrix,
        CbCameraPosition,
        CbOutlineProperty,
        CbLightProperty,
        CbMaterial,
        CbColor,
        CbScreenSpaceMatrix,
        ConstantBufferCount
    };

    enum class eRasterType : uint8_t
    {
        Basic,
        Outline,
        Skybox,
        CullBack,
        RasterCount,
    };

    enum class eSamplerType : uint8_t
    {
        AnisotropicWrap,
        SamplerCount
    };


    enum class eShader : uint8_t
    {
        Outline,
        Skybox,
        Shadow,
        BasicWithShadow,
        RenderToTexture,
        Color,
        ShaderCount
    };

    // RenderTarget, DepthStencil 
    enum class eRenderTarget : uint8_t
    {
        Default,
        Shadow,
        RenderTargetCount
    };

}
