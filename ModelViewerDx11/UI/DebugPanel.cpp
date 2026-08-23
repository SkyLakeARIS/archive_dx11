#include "DebugPanel.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"

namespace ui
{
    DebugPanel::DebugPanel(int16_t originX, int16_t originY, int16_t width, int16_t height)
    {
        renderer::MeshGenerator::CreateScreenPlane(originX, originY, width, height, mMesh);
    }

    DebugPanel::~DebugPanel()
    {
        
    }

    void DebugPanel::Draw(std::vector<renderer::RenderPacket>& commandList)
    {
        for (const auto& subMesh : mMesh.SubMeshes)
        {
            renderer::RenderPacket command = renderer::RenderPacket::MakeCommand(
                mMesh.VertexFormat,
                renderer::GetVertexStrideSize(mMesh.VertexFormat),
                renderer::eBufferUsage::Static,
                false,
                subMesh.VertexRange,
                subMesh.IndexRange,
                subMesh.Material,
                renderer::eRenderTarget::Default,
                XMMatrixIdentity(),
                renderer::eShader::DebugHUD,
                renderer::eRasterType::Basic,
                renderer::eSamplerType::AnisotropicWrap,
                0,
                renderer::ePrimitiveTopology::TriangleStrip,
                false,
                renderer::eDepthStencilState::DepthOffStencilOff,
                false
                );


            commandList.push_back(command);
        }
    }

    void DebugPanel::SetDebugType(HashID texHash, int16_t serialID)
    {
        // MEMO: 단일 메시이므로 즉시 접근
        renderer::SemiMaterial& material = mMesh.SubMeshes.front().Material;
        material.TextureHashes[static_cast<uint8_t>(renderer::eTextureType::Diffuse)] = texHash;
        material.TextureSerials[static_cast<uint8_t>(renderer::eTextureType::Diffuse)] = serialID;
    }
}
