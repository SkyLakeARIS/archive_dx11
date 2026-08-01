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
        bool bUseDynamicBuffer;
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
}
