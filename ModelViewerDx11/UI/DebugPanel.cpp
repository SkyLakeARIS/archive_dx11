#include "DebugPanel.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Primitive/MeshGenerator.h"

namespace ui
{
    DebugPanel::DebugPanel(int16_t originX, int16_t originY, int16_t width, int16_t height)
    {
        renderer::MeshGenerator::CreateScreenPlane(originX, originY, width, height, mMesh);

        renderer::Material& subMeshMaterial = mMesh.SubMeshes.front().Material;
        subMeshMaterial.ShaderType = renderer::eShader::RenderToTexture;
        subMeshMaterial.RasterType = renderer::eRasterType::Basic;
        subMeshMaterial.SamplerType = renderer::eSamplerType::AnisotropicWrap;
        subMeshMaterial.BlendHash = 0;
        subMeshMaterial.TopologyType = renderer::ePrimitiveTopology::TriangleStrip;
        subMeshMaterial.bUseDepthStencil = false;
        subMeshMaterial.MaterialParam = {};
    }

    DebugPanel::~DebugPanel()
    {
        
    }

    void DebugPanel::Draw(renderer::Renderer& renderer)
    {
        renderer::CbWorld cbWorldMat;
        cbWorldMat.Matrix = XMMatrixTranspose(XMMatrixIdentity());
        renderer.UpdateCB(renderer::eCbType::CbWorld, &cbWorldMat);
        renderer.BindCbToVsByType(0, 1, renderer::eCbType::CbWorld);
        renderer.BindCbToVsByType(1, 1, renderer::eCbType::CbScreenSpaceMatrix);

        renderer.BindInputLayoutTo(mMesh.VertexFormat);

        const int16_t strideVertex = GetVertexStrideSize(mMesh.VertexFormat);
        renderer.BindVertexBuffer(strideVertex);
        renderer.BindIndexBuffer();

        for(const auto& subMesh : mMesh.SubMeshes)
        {
            renderer.BindShaderTo(subMesh.Material.ShaderType);
            renderer.BindSamplerToPsByType(0, subMesh.Material.SamplerType);

            if (mType == renderer::eRenderTarget::Shadow)
            {
                renderer.BindShadowTextureToPs(0);
            }
            else
            {
                renderer.BindDefaultTextureToPs(0);
            }
            renderer.DrawIndexed(subMesh.IndexRange.Count, subMesh.IndexRange.StartIndex, subMesh.VertexRange.StartIndex);
        }

        renderer.UnbindTexturePs(0);
    }

    void DebugPanel::SetDebugType(renderer::eRenderTarget type)
    {
        mType = type;
    }
}
