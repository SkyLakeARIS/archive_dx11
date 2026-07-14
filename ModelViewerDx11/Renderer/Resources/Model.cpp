#include "Model.h"
#include "BufferManager.h"
#include "../Renderer.h"
#include "../../Util/Macro.h"
#include "../Importer/ModelImporter.h"

namespace renderer
{
    Model::Model(scene::Camera* camera, BufferManager* bufferManager)
        : mBufferManager(bufferManager)
        , mCenterPosition(0.0f, 0.0f, 0.0f)
        , mMatRotation(XMMatrixIdentity())
        , mMatScale(XMMatrixIdentity())
        , mbHighlight(false)
        , mbActiveEmissive(false)
    {
        ASSERT(camera != nullptr, "do not pass nullptr");

        mMatWorld  = XMMatrixIdentity();
    }

    Model::~Model()
    {
        const int16_t stride = GetVertexStrideSize(mMesh.VertexFormat);
        for(auto& subMesh : mMesh.SubMeshes)
        {
            const int16_t strideIndex = mBufferManager->GetIndexStrideSize();
            mBufferManager->RemoveVertexData(stride, subMesh.SubMeshHash);
            mBufferManager->RemoveIndexData(strideIndex, subMesh.SubMeshHash);
        }
        // TODO: BufferManager를 받지 않고, BufferData 처리할 수 있는 로직이 필요함.
        mBufferManager = nullptr;
    }

    void Model::Draw(renderer::Renderer& renderer)
    {
        renderer.BindInputLayoutTo(eVertexFormat::PTN);

        const uint32 stride = sizeof(VertexPTN);

        renderer.BindVertexBuffer(stride);
        renderer.BindIndexBuffer();

        // outline
        if (mbHighlight)
        {
            renderer.BindRasterStateByType(eRasterType::CullBack);

            renderer.BindShaderTo(eShader::Outline);
            renderer.BindCbToVsByType(0U, 1U, eCbType::CbWorld);
            renderer.BindCbToVsByType(1U, 1U, eCbType::CbOutlineProperty);
            renderer.BindCbToVsByType(2U, 1U, eCbType::CbViewProj);

            for (const auto& subMesh :mMesh.SubMeshes)
            {
                renderer.DrawIndexed(static_cast<uint32_t>(subMesh.IndexRange.Count), subMesh.IndexRange.StartIndex, subMesh.VertexRange.StartIndex);
            }
            // reset for basic draw
            renderer.ClearDepthBuffer();
        }

        renderer.BindRasterStateByType(eRasterType::Basic);

        renderer.BindShaderTo(eShader::BasicWithShadow);

        renderer.BindCbToVsByType(0U, 1U, eCbType::CbWorld);
        renderer.BindCbToVsByType(1U, 1U, eCbType::CbLightViewProjMatrix);
        renderer.BindCbToVsByType(2U, 1U, eCbType::CbLightProperty);
        renderer.BindCbToVsByType(3U, 1U, eCbType::CbCameraPosition);
        renderer.BindCbToVsByType(4U, 1U, eCbType::CbViewProj);

        renderer.BindSamplerToPsByType(0, eSamplerType::AnisotropicWrap);

        renderer.BindCbToPs(0U, 1U, eCbType::CbMaterial);

        renderer.BindShadowTextureToPs(2);

        // Draw
        for (const auto& subMesh : mMesh.SubMeshes)
        {
            renderer.BindTextureToPs(0, subMesh.Material.TextureHashes[static_cast<int8_t>(eTextureType::Diffuse)]);
            if(subMesh.Material.TextureHashes[static_cast<int8_t>(eTextureType::Normal)])
            {
                renderer.BindTextureToPs(1, subMesh.Material.TextureHashes[static_cast<int8_t>(eTextureType::Normal)]);
            }
            CbMaterial cbMaterial;
            ZeroMemory(&cbMaterial, sizeof(CbMaterial));

            memcpy(&cbMaterial, &subMesh.Material.MaterialParam, sizeof(MaterialParameter));
            if (!mbActiveEmissive)
            {
                cbMaterial.Emissive = XMFLOAT3(0.0f, 0.0f, 0.0f);
            }
            renderer.UpdateCB(eCbType::CbMaterial, &cbMaterial);

            renderer.DrawIndexed(static_cast<uint32_t>(subMesh.IndexRange.Count), subMesh.IndexRange.StartIndex, subMesh.VertexRange.StartIndex);
        }

        renderer.UnbindTexturePs(2);
    }

    void Model::DrawShadow(renderer::Renderer& renderer)
    {
        renderer.BindInputLayoutTo(eVertexFormat::P);

        const uint32 stride = sizeof(VertexPTN);

        renderer.BindVertexBuffer(stride);
        renderer.BindIndexBuffer();

        renderer.BindRasterStateByType(eRasterType::Outline);
        renderer.BindShaderTo(eShader::Shadow);

        renderer.BindCbToVsByType(0U, 1U, eCbType::CbWorld);
        renderer.BindCbToVsByType(1U, 1U, eCbType::CbLightViewProjMatrix);

        // Draw
        for (const auto& subMesh : mMesh.SubMeshes)
        {
            renderer.DrawIndexed(static_cast<uint32_t>(subMesh.IndexRange.Count), subMesh.IndexRange.StartIndex, subMesh.VertexRange.StartIndex);
        }
    }

    void Model::Update(renderer::Renderer& renderer)
    {
        CbWorld cbWorld;
        cbWorld.Matrix = XMMatrixTranspose(mMatWorld);

        renderer.UpdateCB(eCbType::CbWorld, &cbWorld);
    }

    void Model::SetMesh(const Mesh& mesh)
    {
        mMesh = std::move(mesh);
    }

    void Model::SetCenterPoint(XMFLOAT4& centerPoint)
    {
        mCenterPosition = XMFLOAT3(centerPoint.x, centerPoint.y, centerPoint.z);
    }

    void Model::SetHighlight(bool bSelection)
    {
        mbHighlight = bSelection;
    }

    XMFLOAT3 Model::GetCenterPoint() const
    {
        XMFLOAT3 pos = mCenterPosition;
        pos.y += 1.0f;
        return pos;
    }
}
