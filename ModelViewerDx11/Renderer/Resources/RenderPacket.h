#pragma once
#include "Material.h"
#include "RenderTypes.h"
#include "../../Util/Macro.h"
#include "../Shader/ShaderManager.h"

namespace renderer
{
    struct SubMesh;
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
            // MEMO:  높은 쪽 <-----------> 낮은 쪽
            // MEMO: 상위 비트 (63-55) | 중간 비트 (54-20) | 하위 비트 (19-0)

            // MEMO: 추후에 작업하면서 필요에 따라 Bit 수, 순서 조정하면서 정답을 찾아가기.

            // MEMO: 렌더타겟이 가장 최상위여야 함. - 물체들이 결국 어느 한 렌더타겟에 그려져야 하므로 렌더타겟에 종속적.
            // MEMO: 뷰포트는 렌더타겟에 종속적으로 판단됨. 그러나 프로젝트에서 사용하지 않으므로 제외.
            // MEMO: 렌더 패스는 최상위 비트 할당. 렌더 패스 타입에 따라 내부적으로 적절한 렌더타켓을 바인드하도록 한다.
            // MEMO: 머티리얼은 우선, 중간 비트를 사용한다. -> 그러나 아직 머티리얼 식별자가 없으므로 제외한다. 조만간 바로 작업 들어가야 함.
            // MEMO: 투명/불투명 여부도 중간 비트를 사용한다. 중간에서 가장 최상위로 둔다.
            // MEMO: 셰이더는 우선 낮은 비트를 사용하되, 중간 비트로 올릴지 고민
            // 현재는 머티리얼당 셰이더 하나와 대응되어 의미 없지만 같은 셰이더를 공유하는 머티리얼이 있다면 대응이 될 수 있는 구조로 판단됨.
            // MEMO: 나머지 렌더 상태는 하위 비트를 쓴다. 현재 구조에 따라서 잘 안 바뀔 것 같은 것을 높은쪽에 둔다.


            constexpr uint64_t RenderPassPriority[] =
            {
                1,
                2,
                0,
            };
            static_assert(sizeof(RenderPassPriority) / sizeof(RenderPassPriority[0]) == static_cast<uint64_t>(eRenderPass::PassCount), "RenderPassPriority와 eRenderPass의 갯수가 서로 맞아야 합니다.");

            uint64_t sortKey = 0;
            // MEMO: 상위 비트 영역
            sortKey |= (RenderPassPriority[static_cast<uint8_t>(command.RenderPass)] << 62); // 2bit
            // MEMO: 중간 비트 영역
            // MEMO: 내림자순이므로, 값이 반전되도록 해야 불투명을 먼저 그림
            sortKey |= static_cast<uint64_t>(command.bTransparency == false) << 54; // 1bit
            sortKey |= static_cast<uint64_t>(command.RenderState.ShaderType) << 50; // 4bit
            sortKey |= static_cast<uint64_t>(command.Material.TextureSerials[static_cast<uint8_t>(eTextureType::Diffuse)]) << 16; // 16bit
            // MEMO: 하위 비트 영역
            sortKey |= static_cast<uint64_t>(command.BufferUsage) << 14; // 1bit
            sortKey |= static_cast<uint64_t>(command.RenderState.DepthStencilState) << 13; // 1bit
            sortKey |= static_cast<uint64_t>(command.RenderState.UseShadowMapUsage) << 11; // 1bit
            sortKey |= static_cast<uint64_t>(command.VertexFormat) << 8; // 3bit
            sortKey |= static_cast<uint64_t>(command.RenderState.RasterType) << 4; // 4bit
            sortKey |= static_cast<uint64_t>(command.RenderState.TopologyType) << 0; // 4 bit

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
