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
            const eVertexFormat vertexFormat,
            const int16_t stride,
            const eBufferUsage bufferUsage,
            const bool bTransparency,
            const BufferRange& vertexRange,
            const BufferRange& indexRange,
            const SemiMaterial& material,
            const eRenderTarget renderTargetType,
            const XMMATRIX& matWorld,
            const eShader shader,
            const eRasterType rasterState,
            const eSamplerType sampler,
            const HashID& blendHash,
            const ePrimitiveTopology topology,
            const bool bUseShadowMap,
            const bool bUseDepthStencil,
            const bool bClearDepthStencilBuffer
        )
        {
            RenderPacket command;
            command.VertexFormat = vertexFormat;
            command.Stride = stride;
            command.BufferUsage = bufferUsage;
            command.bTransparency = bTransparency;
            command.VertexRange = vertexRange;
            command.IndexRange = indexRange;
            command.Material = material;
            command.RenderTargetType = renderTargetType;
            command.MatWorld = matWorld;
            command.RenderState.ShaderType = shader;
            ShaderManager::GetMaterialCbBindingDesc(shader, command.RenderState.CbBindingDesc);
            ShaderManager::GetMaterialTextureBindSlots(shader, command.RenderState.TexBindingSlots);
            ShaderManager::GetMaterialSamplerBindSlot(shader, command.RenderState.SamplerBindingSlot);
            command.RenderState.RasterType = rasterState;
            command.RenderState.SamplerType = sampler;
            command.RenderState.BlendHash = blendHash;
            command.RenderState.TopologyType = topology;
            command.RenderState.UseShadowMapUsage = static_cast<eShadowMapUsage>(bUseShadowMap);
            command.RenderState.DepthStencilUsage = static_cast<eDepthStencilUsage>(bUseDepthStencil);
            command.RenderState.bClearDepthStencilBuffer = bClearDepthStencilBuffer;

            // MEMO: 잘못된 조합 체크. MaterialCb, Texture는 -1이면 사용 안함으로 간주
            ASSERT((sampler != eSamplerType::SamplerCount && command.RenderState.SamplerBindingSlot != -1) || (sampler == eSamplerType::SamplerCount && command.RenderState.SamplerBindingSlot == -1),
                "Sampler가 사용되지만 지정하지 않았거나, Sampler를 지정했지만 사용되지 않습니다. shader(%d), sampler(%d) bindingSlot(%d)", shader, sampler, command.RenderState.SamplerBindingSlot);

            // MEMO: 64bit, 내림차순 정렬(값이 큰 순서로 렌더링)
            // MEMO:  높은 쪽 <-----------> 낮은 쪽
            // MEMO: 상위 비트 (63-55) | 중간 비트 (54-20) | 하위 비트 (19-0)

            // MEMO: 추후에 작업하면서 필요에 따라 Bit 수, 순서 조정하면서 정답을 찾아가기.

            // MEMO: 렌더타겟이 가장 최상위여야 함. - 물체들이 결국 어느 한 렌더타겟에 그려져야 하므로 렌더타겟에 종속적.
            // MEMO: 뷰포트는 렌더타겟에 종속적으로 판단됨. 그러나 프로젝트에서 사용하지 않으므로 제외.
            // MEMO: 패스는 렌더타겟보다 상위여야 할지? 하위여야 할지? - 현재 프로젝트에서는 렌더타겟 == 패스이므로 패스는 무시.
            // MEMO: 머티리얼은 우선, 중간 비트를 사용한다. -> 그러나 아직 머티리얼 식별자가 없으므로 제외한다. 조만간 바로 작업 들어가야 함.
            // MEMO: 투명/불투명 여부도 중간 비트를 사용한다. 중간에서 가장 최상위로 둔다.
            // MEMO: 셰이더는 우선 낮은 비트를 사용하되, 중간 비트로 올릴지 고민
            // 현재는 머티리얼당 셰이더 하나와 대응되어 의미 없지만 같은 셰이더를 공유하는 머티리얼이 있다면 대응이 될 수 있는 구조로 판단됨.
            // MEMO: 나머지 렌더 상태는 하위 비트를 쓴다. 현재 구조에 따라서 잘 안 바뀔 것 같은 것을 높은쪽에 둔다.


            // TODO: improve - 값을 보고 총 몇비트가 필요한지 자동으로 계산하도록 하면 좋을 것 같다. 나중에 한번 고민해보기(컴파일 타임에도 가능한가?)
            constexpr uint64_t RenderTargetPriority[static_cast<uint8_t>(eRenderTarget::RenderTargetCount)] =
            {
                0,
                1,
            };


            // TODO: 머티리얼은 같은지 다른지 구분할 식별자가 필요하다. 우선은 구분하지 않아도 되므로 무시하되, 렌더큐 구조 완료 후 바로 작업이 필요함.
            // MaterialParameter
            // TODO: 해시라서 Bit에 할당하기 애매한 상태. BlendState를 여러 개 대표적으로 쓸 것들만 뽑아서 열거형으로 만들어 사용하는 것으로 변경한다.
            // BlendHash
            // TODO: 텍스처 해시도 동일한 텍스처를 쓰는 드로우콜을 뭉치면 좋을 것 같지만, 그렇게하면 해시가 아니라 다른 ID로 써야할 것 같다.

            // TODO: 설계 문서 대로 비트 위치는 구분해놓는 것이 깔끔할 것 같다. -> 후순위. 64비트로 바꿔야 할 수 있으므로 텍스처/머티리얼 식별자를 먼저 작업하고 대응한다.
            uint64_t sortKey = 0;
            // MEMO: 상위 비트 영역
            sortKey |= (RenderTargetPriority[static_cast<uint8_t>(command.RenderTargetType)] << 63);
            // MEMO: 중간 비트 영역
            // MEMO: 내림자순이므로, 값이 반전되도록 해야 불투명을 먼저 그림
            sortKey |= static_cast<uint64_t>(command.bTransparency == false) << 54;
            sortKey |= static_cast<uint64_t>(command.Material.TextureSerials[static_cast<uint8_t>(eTextureType::Diffuse)]) << 20;
            // MEMO: 하위 비트 영역
            sortKey |= static_cast<uint64_t>(command.BufferUsage) << 18;
            sortKey |= static_cast<uint64_t>(command.RenderState.DepthStencilUsage) << 17;
            sortKey |= static_cast<uint64_t>(command.RenderState.bClearDepthStencilBuffer) << 16;
            sortKey |= static_cast<uint64_t>(command.RenderState.UseShadowMapUsage) << 15;
            sortKey |= static_cast<uint64_t>(command.RenderState.ShaderType) << 11;
            sortKey |= static_cast<uint64_t>(command.VertexFormat) << 8;
            sortKey |= static_cast<uint64_t>(command.RenderState.RasterType) << 4;
            sortKey |= static_cast<uint64_t>(command.RenderState.TopologyType) << 0;

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
        // shadow, normal pass
        eRenderTarget RenderTargetType;
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
        eRenderTarget RenderTargetType = eRenderTarget::RenderTargetCount;
        eShader ShaderType = renderer::eShader::ShaderCount;
        eRasterType RasterType = renderer::eRasterType::RasterCount;
        eSamplerType SamplerType = renderer::eSamplerType::SamplerCount;
        ePrimitiveTopology TopologyType = renderer::ePrimitiveTopology::TopologyCount;
        eDepthStencilUsage DepthStencilUsage = eDepthStencilUsage::UsageCount;
        eShadowMapUsage ShadowMapUsage = eShadowMapUsage::UsageCount;
        int8_t SamplerBindingSlot = -1;
        HashID BlendHash = 0;
    };

    inline bool RenderPacketCompareDecr(const renderer::RenderPacket& lhs, const renderer::RenderPacket& rhs)
    {
        return lhs.SortKey > rhs.SortKey;
    }
}
