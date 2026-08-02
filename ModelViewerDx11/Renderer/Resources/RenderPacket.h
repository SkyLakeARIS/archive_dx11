#pragma once
#include "Material.h"
#include "RenderTypes.h"

namespace renderer
{
    struct SubMesh;
    // MEMO: 일단 현재 필요한 정보들만 모아놓는다.
    struct RenderPacket
    {
        // bind to inputlayout
        eVertexFormat VertexFormat;
        // bind to buffer
        int16_t Stride;
        eBufferUsage BufferUsage;
        bool bTransparency;
        // SubMesh's Ranges
        BufferRange VertexRange;
        BufferRange IndexRange;
        SemiMaterial Material;
        // shadow, normal pass
        eRenderTarget RenderTargetType;
        XMMATRIX MatWorld;
        RenderState RenderState;
        uint32_t SortKey;
    };

    // MEMO: RenderPacket을 사용하지 않고, 캐시가 필요한 상태값들만 명확하게 모아서 처리한다.
    // MEMO: 구조체에서 멤버를 따로 분리하여 유효하지 않은 초기화를 통해 캐시 오염을 막기 위함.
    // MEMO: 캐시되는 상태들은 bool -> 열거형 전환이 필요함.
    struct RenderPacketCache
    {
        int16_t Stride = 0;
        eVertexFormat VertexFormat = eVertexFormat::FormatCount;
        eBufferUsage BufferUsage = eBufferUsage::UsageCount;
        eRenderTarget RenderTargetType = eRenderTarget::RenderTargetCount;
        eShader ShaderType = renderer::eShader::ShaderCount;
        eRasterType RasterType = renderer::eRasterType::RasterCount;
        eSamplerType SamplerType = renderer::eSamplerType::SamplerCount;
        ePrimitiveTopology TopologyType = renderer::ePrimitiveTopology::TopologyCount;
        bool bUseDepthStencil;
        bool bUseShadowMap;
        int8_t SamplerBindingSlot = -1;
        HashID BlendHash = 0;
    };

}
