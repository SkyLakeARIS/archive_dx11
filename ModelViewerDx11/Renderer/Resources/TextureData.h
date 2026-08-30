#pragma once
#include "../GfxPrerequisites.h"
#include "../../Util/Type.h"

namespace renderer
{
    enum class eTextureType : uint8_t
    {
        Diffuse,
        Normal,
        Shadow,
        TextureTypeCount,
    };

    struct TextureData
    {
        ID3D11ShaderResourceView* SRV;
        // MEMO: 텍스처 식별자
        HashID Hash;
        // MEMO: SortKey 위한 id.
        int16_t SerialID;
    };
}
