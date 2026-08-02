#pragma once
#include "../../framework.h"
#include "TextureData.h"

namespace renderer
{
    enum class eSamplerType : uint8_t;
    enum class eRasterType : uint8_t;
    enum class eShader : uint8_t;
}

namespace renderer
{
    enum class eBufferUsage : uint8_t
    {
        Static,
        Dynamic,
        UsageCount
    };

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
        // TODO: improve - outlineProperty는 왜 float3로 했는지? -> CbFloat으로 분리하기
    } CbCameraPosition, CbOutlineProperty, CbColor;

    typedef struct CbTwoVec4
    {
        XMFLOAT4    First;
        XMFLOAT4    Second;
    }CbLightProperty;

    // TODO: cleanup - 관련 코드들 ShaderManager로 이동
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
        CbOrthoMatrix,
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
        DebugHUD,
        ShaderCount
    };

    // RenderTarget, DepthStencil 
    enum class eRenderTarget : uint8_t
    {
        Default,
        Shadow,
        RenderTargetCount
    };

    struct MaterialCbBinding
    {
        eCbType Type;
        bool bBindPixelShader;
        int8_t BindSlot;
    };

    struct RenderState
    {
        // MEMO: 셰이더
        eShader ShaderType;
        // MEMO: 정의된 머티리얼, 셰이더 파일로부터 정보를 얻어와야 하지만,
        // 프로젝트가 외부 fbx를 읽기 때문에 별도 파일은 만들지 않고, 엔진에서 어느 정도 하드코드하는 형식으로 선택.
        // 프로젝트가 고도화되었을 때 파일 형식으로 갈지는 그때 가서 고민하기로
        // MEMO: 머티리얼을 생성할 때 셰이더 매니저로부터 얻어오도록
        MaterialCbBinding CbBindingDesc;
        int8_t TexBindingSlots[static_cast<uint8_t>(eTextureType::TextureTypeCount)];
        int8_t SamplerBindingSlot;
        // MEMO: 렌더 상태
        eRasterType RasterType;
        eSamplerType SamplerType;
        ePrimitiveTopology TopologyType;
        HashID BlendHash;
        // TODO: improve - 현재 옵션이 Skybox 전용으로만 존재하므로 확장이 필요함.
        bool bUseDepthStencil;
        bool bUseShadowMap;
        bool bClearDepthStencilBuffer;
    };
}
