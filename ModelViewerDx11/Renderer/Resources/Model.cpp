#include "Model.h"
#include "BufferManager.h"
#include "../../Util/Macro.h"
#include "../Resources/RenderPacket.h"
#include "../Shader/ShaderManager.h"

namespace renderer
{
    Model::Model(scene::Camera* camera, BufferManager* bufferManager)
        : mBufferManager(bufferManager)
        , mMesh()
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

    void Model::Draw(std::vector<renderer::RenderPacket>& commandList)
    {
        // outline
        if (mbHighlight)
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
                    mMatWorld,
                    renderer::eShader::Outline,
                    renderer::eRasterType::CullBack,
                    renderer::eSamplerType::AnisotropicWrap,
                    0,
                    renderer::ePrimitiveTopology::Triangles,
                    false,
                    true,
                    true
                );

                if (!mbActiveEmissive)
                {
                    command.Material.MaterialParam.Emissive = XMFLOAT3(0.0f, 0.0f, 0.0f);
                }

                commandList.push_back(command);
            }
        }


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
                mMatWorld,
                renderer::eShader::BasicWithShadow,
                renderer::eRasterType::Basic,
                renderer::eSamplerType::AnisotropicWrap,
                0,
                renderer::ePrimitiveTopology::Triangles,
                true,
                false,
                false
            );

            if (!mbActiveEmissive)
            {
                command.Material.MaterialParam.Emissive = XMFLOAT3(0.0f, 0.0f, 0.0f);
            }

            commandList.push_back(command);
        }
    }

    void Model::DrawShadow(std::vector<renderer::RenderPacket>& commandList)
    {

        for (const auto& subMesh : mMesh.SubMeshes)
        {
            renderer::RenderPacket command = renderer::RenderPacket::MakeCommand(
                eVertexFormat::P,
                renderer::GetVertexStrideSize(mMesh.VertexFormat),
                renderer::eBufferUsage::Static,
                false,
                subMesh.VertexRange,
                subMesh.IndexRange,
                subMesh.Material,
                renderer::eRenderTarget::Shadow,
                mMatWorld,
                renderer::eShader::Shadow,
                renderer::eRasterType::Outline,
                renderer::eSamplerType::SamplerCount,
                0,
                renderer::ePrimitiveTopology::Triangles,
                false,
                false,
                false
            );

            commandList.push_back(command);
        }
    }

    void Model::Update()
    {
        // MEMO: 전치 안 해도 되지만, 나중에 필요해질 테니 까먹지 않도록 미리 구성해둠.
        mMatWorld = XMMatrixTranspose(XMMatrixIdentity());
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
