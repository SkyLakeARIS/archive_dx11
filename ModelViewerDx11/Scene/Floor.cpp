#include "Floor.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Util/Macro.h"

namespace scene
{
    Floor::Floor(XMFLOAT2 startPoint, uint32_t gapEachLine, uint32_t numLineX, uint32_t numLineY)
    {
        ASSERT(numLineX >= 2, "numLineX must be 2 or greater");
        ASSERT(numLineY >= 2, "numLineY must be 2 or greater");
        ASSERT(gapEachLine >= 1, "gapEachLine must be 1 or greater");

        renderer::MeshGenerator::CreateGrid(startPoint, numLineX, numLineY, gapEachLine, mMesh);

        renderer::Material& subMeshMaterial = mMesh.SubMeshes.front().Material;
        subMeshMaterial.ShaderType = renderer::eShader::Color;
        subMeshMaterial.RasterType = renderer::eRasterType::Basic;
        // no sampler, blend
        subMeshMaterial.BlendHash = 0;
        subMeshMaterial.TopologyType = renderer::ePrimitiveTopology::TriangleStrip;
        subMeshMaterial.bUseDepthStencil = false;
        subMeshMaterial.MaterialParam = {};
    }

    Floor::~Floor()
    {
        // TODO: 추가한 BufferData 처리할 수 있는 로직이 필요함. - (종료될 떄 처리되기 때문에 당장 문제는 없음)
    }

    void Floor::Draw(renderer::Renderer& renderer)
    {

        renderer.BindInputLayoutTo(mMesh.VertexFormat);
        const int16_t stride = renderer::GetVertexStrideSize(mMesh.VertexFormat);
        renderer.BindVertexBuffer(stride);
        for(const auto& subMesh : mMesh.SubMeshes)
        {
            renderer.BindRasterStateByType(subMesh.Material.RasterType);
            renderer.BindShaderTo(subMesh.Material.ShaderType);

            renderer::CbWorld cbWorld;
            cbWorld.Matrix = XMMatrixIdentity();
            renderer.UpdateCB(renderer::eCbType::CbWorld, &cbWorld);

            renderer.BindCbToVsByType(0, 1, renderer::eCbType::CbWorld);
            renderer.BindCbToVsByType(1, 1, renderer::eCbType::CbViewProj);
            renderer.BindCbToPs(0, 1, renderer::eCbType::CbColor);

            renderer::CbColor cbColor = {};
            cbColor.Float3 = XMFLOAT3(0.0, 1.0, 0.0);
            renderer.UpdateCB(renderer::eCbType::CbColor, &cbColor);

            D3D11_PRIMITIVE_TOPOLOGY orgTopology;
            renderer.GetCurrentPrimitiveTopology(orgTopology);

            renderer.BindPrimitiveTopologyByType(subMesh.Material.TopologyType);

            renderer.Draw(subMesh.VertexRange.Count, subMesh.VertexRange.StartIndex);

            renderer.BindPrimitiveTopologyTo(orgTopology);
        }
    }
}
