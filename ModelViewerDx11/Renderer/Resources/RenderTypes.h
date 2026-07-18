#pragma once
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
