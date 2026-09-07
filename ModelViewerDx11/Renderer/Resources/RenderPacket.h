#pragma once
#include "Material.h"
#include "RenderTypes.h"
#include "../../Util/Macro.h"
#include "../Shader/ShaderManager.h"

namespace renderer
{
    struct SubMesh;

    template<typename  T>
    constexpr uint64_t GetBitCount(T type)
    {
        int32_t bit = 1;
        uint64_t count = 0;
        while (bit <= static_cast<uint32_t>(type))
        {
            bit <<= 1;
            ++count;
        }
        return count;
    }

    // MEMO: 일단 현재 필요한 정보들만 모아놓는다.
    struct RenderPacket
    {
        // MEMO: 렌더패킷 생성 시 실수로 놓치는 필드가 없도록 하게끔 강제하기 위한 수단으로 팩토리 함수 사용
        // MEMO: DepthStencil, ShadowMap은 쓸지 안쓸지만 결정하므로 생성할 때에는 기존처럼 bool 타입 유지 (열거형은 상태 캐시용이므로)
        static RenderPacket MakeCommand(
            const eVertexFormat      vertexFormat,
            const int16_t            stride,
            const eBufferUsage       bufferUsage,
            const bool               bTransparency,
            const BufferRange&       vertexRange,
            const BufferRange&       indexRange,
            const SemiMaterial&      material,
            const eRenderPass        renderPass,
            const XMMATRIX&          matWorld,
            const eShader            shader,
            const eRasterType        rasterState,
            const eSamplerType       sampler,
            const eBlendState        blendState,
            const ePrimitiveTopology topology,
            const bool               bUseShadowMap,
            eDepthStencilState       depthStencilState
            )
        {
            ASSERT(vertexFormat != eVertexFormat::FormatCount, "vertexFormat 값이 설정되지 않음. 반드시 설정되어야 합니다. passed(%d)", static_cast<uint8_t>(vertexFormat));
            ASSERT(stride > 0 , "stride 값이 올바르지 않음. stride > 0 이어야 합니다. passed(%d)", stride);
            ASSERT(bufferUsage != eBufferUsage::UsageCount, "bufferUsage는 반드시 Static/Dynamic 중 하나로 지정되어야 함.");
            ASSERT(renderPass != eRenderPass::PassCount, "renderPass 값이 설정되지 않음. 반드시 설정되어야 합니다. passed(%d)", static_cast<uint8_t>(renderPass));
            ASSERT(shader != eShader::ShaderCount, "shader 값이 설정되지 않음. 반드시 설정되어야 합니다. passed(%d)", static_cast<uint8_t>(shader));
            ASSERT(rasterState != eRasterType::RasterCount, "rasterState 값이 설정되지 않음. 반드시 설정되어야 합니다. passed(%d)", static_cast<uint8_t>(rasterState));
            ASSERT(blendState != eBlendState::StateCount, "blendState 값이 설정되지 않음. 반드시 설정되어야 합니다. passed(%d)", static_cast<uint8_t>(blendState));
            // MEMO: sampler는 텍스처 여부에 따라 다르므로 우선 대상 제외
            ASSERT(topology != ePrimitiveTopology::TopologyCount, "topology 값이 설정되지 않음. 반드시 설정되어야 합니다. passed(%d)", static_cast<uint8_t>(topology));
            ASSERT(depthStencilState != eDepthStencilState::StateCount, "depthStencilState 값이 설정되지 않음. 사용하지 않으려면 DepthOff를 지정해야 합니다. passed(%d)", static_cast<uint8_t>(depthStencilState));
            RenderPacket command;
            command.VertexFormat = vertexFormat;
            command.Stride = stride;
            command.BufferUsage = bufferUsage;
            command.bTransparency = bTransparency;
            command.VertexRange = vertexRange;
            command.IndexRange = indexRange;
            command.Material = material;
            command.RenderPass = renderPass;
            command.MatWorld = matWorld;
            command.RenderState.ShaderType = shader;
            ShaderManager::GetMaterialCbBindingDesc(shader, command.RenderState.CbBindingDesc);
            ShaderManager::GetMaterialTextureBindSlots(shader, command.RenderState.TexBindingSlots);
            ShaderManager::GetMaterialSamplerBindSlot(shader, command.RenderState.SamplerBindingSlot);
            command.RenderState.RasterType = rasterState;
            command.RenderState.SamplerType = sampler;
            command.RenderState.BlendState = blendState;
            command.RenderState.TopologyType = topology;
            command.RenderState.UseShadowMapUsage = static_cast<eShadowMapUsage>(bUseShadowMap);
            command.RenderState.DepthStencilState = depthStencilState;

            // MEMO: 잘못된 조합 체크. MaterialCb, Texture는 -1이면 사용 안함으로 간주
            ASSERT((sampler != eSamplerType::SamplerCount && command.RenderState.SamplerBindingSlot != -1) || (sampler == eSamplerType::SamplerCount && command.RenderState.SamplerBindingSlot == -1),
                "Sampler가 사용되지만 지정하지 않았거나, Sampler를 지정했지만 사용되지 않습니다. shader(%d), sampler(%d) bindingSlot(%d)", shader, sampler, command.RenderState.SamplerBindingSlot);


            // MEMO: 64bit, 내림차순 정렬(값이 큰 순서로 렌더링)
            constexpr uint64_t RenderPassPriority[] =
            {
                2, // Main(GPass)
                3, // Shadow
                0, // UI
                1, // Deferred
            };
            static_assert(sizeof(RenderPassPriority) / sizeof(RenderPassPriority[0]) == static_cast<uint64_t>(eRenderPass::PassCount), "RenderPassPriority와 eRenderPass의 갯수가 서로 맞아야 합니다.");

            // MEMO: 이 함수 전용으로 사용
            struct SortKeyField
            {
                uint64_t Value;
                uint64_t BitWidth;
            };

            // MEMO: 상위부터 차례대로 잘라서 사용하도록 개선.
            // MEMO: 순서만 지켜서 레이아웃에 추가하면 나머지 필요한 비트 수와 위치는 자동으로 계산되어 SortKey를 생성.
            const SortKeyField SortKeyLayout[] =
            {
                // MEMO: 상위 비트 영역
                {RenderPassPriority[static_cast<uint8_t>(command.RenderPass)], GetBitCount(eRenderPass::PassCount)},
                // MEMO: 중간 비트 영역
                {command.bTransparency == false, 1},
                {static_cast<uint64_t>(command.RenderState.ShaderType), GetBitCount(eShader::ShaderCount)},
                {static_cast<uint64_t>(command.Material.TextureSerials[static_cast<uint8_t>(eTextureType::Diffuse)]), 16},
                // MEMO: 하위 비트 영역
                {static_cast<uint64_t>(command.BufferUsage), GetBitCount(eBufferUsage::UsageCount)},
                {static_cast<uint64_t>(command.RenderState.DepthStencilState), GetBitCount(eDepthStencilState::StateCount)},
                {static_cast<uint64_t>(command.RenderState.UseShadowMapUsage), GetBitCount(eShadowMapUsage::UsageCount)},
                {static_cast<uint64_t>(command.VertexFormat), GetBitCount(eVertexFormat::FormatCount)},
                {static_cast<uint64_t>(command.RenderState.RasterType), GetBitCount(eRasterType::RasterCount)},
                {static_cast<uint64_t>(command.RenderState.TopologyType), GetBitCount(ePrimitiveTopology::TopologyCount)},
            };


#ifdef _DEBUG
            // MEMO: 사용된 Bit들을 전부 합했을 때 자료형을 넘는지 검사
            uint64_t validBitCount = 0;
            for(auto& element : SortKeyLayout)
            {
                validBitCount += element.BitWidth;
            }
            ASSERT(validBitCount <= 64, "SortKey가 표현할 수 있는 비트 수를 넘었습니다. 계산된 비트 사용량(%llu)", validBitCount);
#endif

            // MEMO: SortKey를 생성
            uint64_t bitShiftCursor = 64;
            uint64_t sortKey = 0;
            for (auto& element : SortKeyLayout)
            {
                bitShiftCursor -= element.BitWidth;
                sortKey |= element.Value << bitShiftCursor;
            }

            command.SortKey = sortKey;

            return command;
        };
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
        eRenderPass RenderPass;
        XMMATRIX MatWorld;
        RenderState RenderState;
        uint64_t SortKey;
    };

    // MEMO: RenderPacket을 사용하지 않고, 캐시가 필요한 상태값들만 명확하게 모아서 처리한다.
    // MEMO: 구조체에서 멤버를 따로 분리하여 유효하지 않은 초기화를 통해 캐시 오염을 막기 위함.
    struct RenderPacketCache
    {
        int16_t Stride = 0;
        eVertexFormat VertexFormat = eVertexFormat::FormatCount;
        eBufferUsage BufferUsage = eBufferUsage::UsageCount;
        eRenderPass RenderPass = eRenderPass::PassCount;
        eRenderTarget RenderTarget = eRenderTarget::RenderTargetCount;
        eShader ShaderType = renderer::eShader::ShaderCount;
        eRasterType RasterType = renderer::eRasterType::RasterCount;
        eSamplerType SamplerType = renderer::eSamplerType::SamplerCount;
        ePrimitiveTopology TopologyType = renderer::ePrimitiveTopology::TopologyCount;
        eDepthStencilState DepthStencilUsage = eDepthStencilState::StateCount;
        eShadowMapUsage ShadowMapUsage = eShadowMapUsage::UsageCount;
        int8_t SamplerBindingSlot = -1;
        eBlendState BlendState = eBlendState::StateCount;
    };

    inline bool RenderPacketCompareDecr(const renderer::RenderPacket& lhs, const renderer::RenderPacket& rhs)
    {
        return lhs.SortKey > rhs.SortKey;
    }
}
