#pragma once
#include <memory>
#include "../../Util/Define.h"
#include "../Resources/Material.h"
#include "../Resources/VertexType.h"

namespace renderer
{
    struct ImportedTextureData
    {
        int8_t FilePath[util::MAX_PATH_LENGTH];
        HashID TextureHash;
        eTextureType TextureType;
    };

    struct ImportedMeshData
    {
        int8_t MeshName[util::MAX_NAME_LENGTH];
        std::unique_ptr<VertexPTN[]> VertexBuffer;
        uint32_t VertexCount;
        std::unique_ptr<uint32_t[]> IndexBuffer;
        uint32_t IndexCount;
        MaterialFactors MaterialParam;
        XMFLOAT3 MinBound;
        XMFLOAT3 MaxBound;
        ImportedTextureData Textures[static_cast<int32_t>(eTextureType::TextureTypeCount)];
    };

    struct ImportedModelContainer
    {
        HashID ModelHash;
        std::vector<ImportedMeshData> SubMeshes;
    };
}
