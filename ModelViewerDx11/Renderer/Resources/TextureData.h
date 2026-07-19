#pragma once
#include "../../framework.h"

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
        HashID Hash;
    };
}
