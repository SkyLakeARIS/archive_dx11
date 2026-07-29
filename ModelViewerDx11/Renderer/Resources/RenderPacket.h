#pragma once
#include "Material.h"
#include "RenderTypes.h"

namespace renderer
{
    struct SubMesh;
    // MEMO: 일단 현재 필요한 정보들만 모아놓는다.
    struct RenderPacket
    {
        eVertexFormat VertexFormat;
        // SubMesh's Ranges
        BufferRange VertexRange;
        BufferRange IndexRange;
        bool bUseDynamicBuffer;
        SemiMaterial Material;
        // shadow, normal pass
        eRenderTarget RenderTargetType;
        XMMATRIX MatWorld;
        RenderState RenderState;
        uint32_t SortKey;
    };
}
