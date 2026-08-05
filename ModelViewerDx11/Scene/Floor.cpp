#include "Floor.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"
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
            subMesh.Material.Factors.Diffuse = XMFLOAT3(0.0f, 1.0f, 0.0f);
        }
    }

    Floor::~Floor()
    {
        // TODO: 추가한 BufferData 처리할 수 있는 로직이 필요함. - (종료될 떄 처리되기 때문에 당장 문제는 없음)
    }

    void Floor::Draw(std::vector<renderer::RenderPacket>& commandList)
    {
        XMMATRIX mat = XMMatrixIdentity();
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
                mat,
                renderer::eShader::Color,
                renderer::eRasterType::Basic,
                renderer::eSamplerType::SamplerCount,
                0,
                renderer::ePrimitiveTopology::TriangleStrip,
                false,
                false,
                false
            );

            commandList.push_back(command);
        }
    }
}
