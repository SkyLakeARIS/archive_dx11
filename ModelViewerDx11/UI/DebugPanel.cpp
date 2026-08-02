#include "DebugPanel.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"
#include "../Renderer/Shader/ShaderManager.h"

namespace ui
{
    DebugPanel::DebugPanel(int16_t originX, int16_t originY, int16_t width, int16_t height)
        : mType()
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
                true,
                false,
                false
                );

            // FIXME: Shadow 텍스처를 텍스처로 쓴다는 것을 알려줄만한 것이 없음. 이 시스템을 만들어둬야 여러 디버그 텍스처를 지원할 수 있음
            // Shadow 텍스처도 해시나 식별자를 가지도록 하고 가져올 수 있도록 해야한다.
            if (mType == renderer::eRenderTarget::Shadow)
            {
                // 우선은 해당 용도가 Shadow 텍스처 확인용이므로 편법으로 우회해서 쓴다.
                command.RenderState.TexBindingSlots[static_cast<uint8_t>(renderer::eTextureType::Shadow)] = command.RenderState.TexBindingSlots[static_cast<uint8_t>(renderer::eTextureType::Diffuse)];
            }
            else
            {
                // TODO: 위와 같은 문제로 기본 텍스처도  사용할 수 없는 상태.
            }

            commandList.push_back(command);
        }
    }

    void DebugPanel::SetDebugType(renderer::eRenderTarget type)
    {
        mType = type;
    }
}
