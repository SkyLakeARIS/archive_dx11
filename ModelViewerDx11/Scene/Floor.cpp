#include "Floor.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"
#include "../Renderer/Shader/ShaderManager.h"
#include "../Util/Macro.h"

namespace scene
{
    Floor::Floor(XMFLOAT2 startPoint, uint32_t gapEachLine, uint32_t numLineX, uint32_t numLineY)
    {
        ASSERT(numLineX >= 2, "numLineX must be 2 or greater");
        ASSERT(numLineY >= 2, "numLineY must be 2 or greater");
        ASSERT(gapEachLine >= 1, "gapEachLine must be 1 or greater");

        renderer::MeshGenerator::CreateGrid(startPoint, numLineX, numLineY, gapEachLine, mMesh);
        for (auto& subMesh : mMesh.SubMeshes)
        {
            subMesh.Material.MaterialParam.Diffuse = XMFLOAT3(0.0f, 1.0f, 0.0f);
        }
    }

    Floor::~Floor()
    {
        // TODO: 추가한 BufferData 처리할 수 있는 로직이 필요함. - (종료될 떄 처리되기 때문에 당장 문제는 없음)
    }

    void Floor::Draw(std::vector<renderer::RenderPacket>& commandList)
    {
        renderer::RenderPacket command = {};
        command.VertexFormat = mMesh.VertexFormat;
        command.Stride = GetVertexStrideSize(mMesh.VertexFormat);
        command.BufferUsage = renderer::eBufferUsage::Static;
        command.bTransparency = false;
        command.RenderTargetType = renderer::eRenderTarget::Default;
        command.MatWorld = XMMatrixIdentity();
        command.RenderState.ShaderType = renderer::eShader::Color;
        renderer::ShaderManager::GetMaterialCbBindingDesc(command.RenderState.ShaderType, command.RenderState.CbBindingDesc);
        renderer::ShaderManager::GetMaterialTextureBindSlots(command.RenderState.ShaderType, command.RenderState.TexBindingSlots);
        renderer::ShaderManager::GetMaterialSamplerBindSlot(command.RenderState.ShaderType, command.RenderState.SamplerBindingSlot);
        command.RenderState.RasterType = renderer::eRasterType::Basic;
        command.RenderState.SamplerType = renderer::eSamplerType::SamplerCount;
        command.RenderState.BlendHash = 0;
        command.RenderState.TopologyType = renderer::ePrimitiveTopology::TriangleStrip;
        command.RenderState.bUseDepthStencil = false;
        command.RenderState.bUseShadowMap = false;
        command.RenderState.bClearDepthStencilBuffer = false;

        for (const auto& subMesh : mMesh.SubMeshes)
        {
            command.VertexRange = subMesh.VertexRange;
            command.IndexRange = subMesh.IndexRange;
            command.Material = subMesh.Material;
            commandList.push_back(command);
        }
    }
}
