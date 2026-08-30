#include "Sky.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"
#include "../Renderer/Resources/TextureManager.h"

namespace scene
{
    Sky::Sky()
        : mMesh()
        , mWorld(XMMatrixIdentity())
    {
    }

    Sky::~Sky()
    {}

    HRESULT Sky::Initialize(uint32_t latLines, uint32_t lonLines, renderer::TextureManager* const texManager)
    {
        renderer::MeshGenerator::CreateSphere(latLines, lonLines, mMesh);



        const int8_t* const filePath = reinterpret_cast<const int8_t*>("./AssetData/textures/skybox.dds");
        HashID texDiffuseHash = 0;
        texManager->AddTextureDDS(filePath, texDiffuseHash);
        mMesh.SubMeshes.front().Material.TextureHashes[static_cast<int8_t>(renderer::eTextureType::Diffuse)] = texDiffuseHash;
        mMesh.SubMeshes.front().Material.TextureSerials[static_cast<int8_t>(renderer::eTextureType::Diffuse)] = texManager->GetTextureSerial(texDiffuseHash);
        return S_OK;
    }

    void Sky::SubmitCommand(std::vector<renderer::RenderPacket>& commandList)
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
                renderer::eRenderPass::Main,
                mWorld,
                renderer::eShader::Skybox,
                renderer::eRasterType::Skybox,
                renderer::eSamplerType::AnisotropicWrap,
                renderer::eBlendState::Opaque,
                renderer::ePrimitiveTopology::Triangles,
                false,
                renderer::eDepthStencilState::DepthOnMaskAllCompLessEqual,
                false
            );

            commandList.push_back(command);
        }
    }

    void Sky::Update(const XMFLOAT3& cameraPosition)
    {
        // update
        constexpr float SKY_SCALE_SIZE = 100.0f;
        const XMMATRIX matScale = XMMatrixScaling(SKY_SCALE_SIZE, SKY_SCALE_SIZE, SKY_SCALE_SIZE);
        const XMMATRIX matTranslate = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

        mWorld = XMMatrixTranspose(matScale * matTranslate);

    }
}
