#include "Model.h"
#include "BufferManager.h"
#include "../Resources/RenderPacket.h"
#include "../../Util/Macro.h"
#include "../Shader/ShaderManager.h"

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

    void Model::Draw(std::vector<renderer::RenderPacket>& commandList)
    {
        RenderPacket command = {};
        command.VertexFormat = mMesh.VertexFormat;
        command.Stride = GetVertexStrideSize(mMesh.VertexFormat);
        command.bUseDynamicBuffer = false;
        command.RenderTargetType = eRenderTarget::Default;
        command.MatWorld = mMatWorld;

        // outline
        if (mbHighlight)
        {
            command.RenderState.ShaderType = eShader::Outline;
            renderer::ShaderManager::GetMaterialCbBindingDesc(command.RenderState.ShaderType, command.RenderState.CbBindingDesc);
            renderer::ShaderManager::GetMaterialTextureBindSlots(command.RenderState.ShaderType, command.RenderState.TexBindingSlots);
            renderer::ShaderManager::GetMaterialSamplerBindSlot(command.RenderState.ShaderType, command.RenderState.SamplerBindingSlot);
            command.RenderState.RasterType = eRasterType::CullBack;
            command.RenderState.SamplerType = eSamplerType::AnisotropicWrap;
            command.RenderState.TopologyType = ePrimitiveTopology::Triangles;
            command.RenderState.BlendHash = 0;
            command.RenderState.bUseShadowMap = false;
            command.RenderState.bUseDepthStencil = true;
            command.RenderState.bClearDepthStencilBuffer = true;

            for (const auto& subMesh : mMesh.SubMeshes)
            {
                command.VertexRange = subMesh.VertexRange;
                command.IndexRange = subMesh.IndexRange;
                command.Material = subMesh.Material;

                if (!mbActiveEmissive)
                {
                    command.Material.MaterialParam.Emissive = XMFLOAT3(0.0f, 0.0f, 0.0f);
                }

                commandList.push_back(command);
            }
            commandList.push_back(command);
        }

        // TODO: ResourceManager에 있는 하드코드가 여기로 이동된 셈. - 나중에 조금이나마 줄일 수 있는 방안이 있을지 고민은 해보고 코멘트 지우자.
        command.RenderState.ShaderType = eShader::BasicWithShadow;
        renderer::ShaderManager::GetMaterialCbBindingDesc(command.RenderState.ShaderType, command.RenderState.CbBindingDesc);
        renderer::ShaderManager::GetMaterialTextureBindSlots(command.RenderState.ShaderType, command.RenderState.TexBindingSlots);
        renderer::ShaderManager::GetMaterialSamplerBindSlot(command.RenderState.ShaderType, command.RenderState.SamplerBindingSlot);
        command.RenderState.RasterType = eRasterType::Basic;
        command.RenderState.SamplerType = eSamplerType::AnisotropicWrap;
        command.RenderState.TopologyType = ePrimitiveTopology::Triangles;
        command.RenderState.BlendHash = 0;
        command.RenderState.bUseShadowMap = true;
        command.RenderState.bUseDepthStencil = false;
        command.RenderState.bClearDepthStencilBuffer = false;

        for (const auto& subMesh : mMesh.SubMeshes)
        {
            command.VertexRange = subMesh.VertexRange;
            command.IndexRange = subMesh.IndexRange;
            command.Material = subMesh.Material;

            if (!mbActiveEmissive)
            {
                command.Material.MaterialParam.Emissive = XMFLOAT3(0.0f, 0.0f, 0.0f);
            }

            commandList.push_back(command);
        }
    }

    void Model::DrawShadow(std::vector<renderer::RenderPacket>& commandList)
    {
        RenderPacket command = {};
        command.VertexFormat = eVertexFormat::P;
        command.Stride = GetVertexStrideSize(mMesh.VertexFormat);
        command.bUseDynamicBuffer = false;
        command.RenderTargetType = eRenderTarget::Shadow;
        command.RenderState.ShaderType = eShader::Shadow;
        renderer::ShaderManager::GetMaterialCbBindingDesc(command.RenderState.ShaderType, command.RenderState.CbBindingDesc);
        renderer::ShaderManager::GetMaterialTextureBindSlots(command.RenderState.ShaderType, command.RenderState.TexBindingSlots);
        renderer::ShaderManager::GetMaterialSamplerBindSlot(command.RenderState.ShaderType, command.RenderState.SamplerBindingSlot);
        command.RenderState.TopologyType = ePrimitiveTopology::Triangles;
        command.MatWorld = mMatWorld;
        command.RenderState.RasterType = eRasterType::Outline;

        for (const auto& subMesh : mMesh.SubMeshes)
        {
            command.VertexRange = subMesh.VertexRange;
            command.IndexRange = subMesh.IndexRange;
            command.Material = subMesh.Material;
            commandList.push_back(command);
        }
    }

    void Model::Update(renderer::Renderer& renderer)
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
