#include "ResourceManager.h"
#include "BufferManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "../../Util/Macro.h"
#include "../../Util/Util.h"
#include "../Importer/ImportedModelData.h"
#include "../Importer/ModelImporter.h"

namespace renderer
{
    ResourceManager::ResourceManager(ID3D11Device* const device, TextureManager* const textureManager, ModelImporter* const importer, BufferManager* const bufferManager)
        : mDevice(device)
        , mTextureManager(textureManager)
        , mModelImporter(importer)
        , mBufferManager(bufferManager)
    {
        ASSERT(device != nullptr, "device is nullptr");
        ASSERT(textureManager != nullptr, "textureManager is nullptr");
        ASSERT(importer != nullptr, "importer is nullptr");
        ASSERT(bufferManager != nullptr, "bufferManager is nullptr");
    }

    ResourceManager::~ResourceManager()
    {
        mDevice = nullptr;
        mTextureManager = nullptr;
        mModelImporter = nullptr;
        mBufferManager = nullptr;
    }

    void ResourceManager::LoadModel(const int8_t* const filePath, Model* const outModel)
    {
        ASSERT(filePath != nullptr, "invalid path. (filePath is nullptr) ");
        ASSERT(*filePath != ' ', "invalid path. (filePath may empty string) ");
        ASSERT(*filePath != '\0', "invalid path. (filePath may empty(null) string) ");

        HashID modelHash = 0;
        ImportedModelContainer modelContainer;
        mModelImporter->LoadFbxModel(filePath, modelHash, modelContainer);

        Mesh newMesh = {};
        newMesh.SubMeshes.reserve(modelContainer.SubMeshes.size());
        newMesh.VertexFormat = eVertexFormat::PTN;
        const int16_t strideVertex = GetVertexStrideSize(newMesh.VertexFormat);
        const int16_t strideIndex = mBufferManager->GetIndexStrideSize();

        int32_t totalVertexCount = 0;
        int32_t totalIndexCount = 0;
        for(auto& subMesh : modelContainer.SubMeshes)
        {
            SubMesh newSubMesh = {};

            newSubMesh.SubMeshHash = util::GetDjb2Hash(subMesh.MeshName);
            mBufferManager->AddVertex(reinterpret_cast<int8_t*>(subMesh.VertexBuffer.get()), strideVertex * subMesh.VertexCount, newSubMesh.SubMeshHash, strideVertex, newSubMesh.VertexRange);

            mBufferManager->AddIndex(reinterpret_cast<int8_t*>(subMesh.IndexBuffer.get()), strideIndex * subMesh.IndexCount, newSubMesh.SubMeshHash, strideIndex, newSubMesh.IndexRange);

            totalVertexCount += newSubMesh.VertexRange.Count;
            totalIndexCount += newSubMesh.IndexRange.Count;
            (void)memcpy(newSubMesh.SubMeshName, subMesh.MeshName, util::MAX_NAME_LENGTH);

            newSubMesh.Material.Factors = std::move(subMesh.MaterialParam);


            for (int32_t tex = 0; tex < static_cast<int32_t>(eTextureType::TextureTypeCount); ++tex)
            {
                if(subMesh.Textures[tex].TextureHash)
                {
                    mTextureManager->AddTexture(subMesh.Textures[tex].FilePath, subMesh.Textures[tex].TextureHash);

                    newSubMesh.Material.TextureHashes[tex] = subMesh.Textures[tex].TextureHash;
                    newSubMesh.Material.TextureSerials[tex] = mTextureManager->GetTextureSerial(subMesh.Textures[tex].TextureHash);
                }
            }
            newMesh.SubMeshes.push_back(std::move(newSubMesh));
        }

        newMesh.VertexRange.StartIndex = newMesh.SubMeshes.front().VertexRange.StartIndex;
        newMesh.VertexRange.Count = totalVertexCount;

        newMesh.IndexRange.StartIndex = newMesh.SubMeshes.front().IndexRange.StartIndex;
        newMesh.IndexRange.Count = totalIndexCount;

        outModel->SetMesh(newMesh);
        outModel->SetCenterPoint(modelContainer.CenterPoint);
    }

}
