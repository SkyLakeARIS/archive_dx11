#include "Sky.h"
#include "Camera.h"
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

    void Sky::Draw(std::vector<renderer::RenderPacket>& commandList)
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
                mWorld,
                renderer::eShader::Skybox,
                renderer::eRasterType::Skybox,
                renderer::eSamplerType::AnisotropicWrap,
                0,
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
        XMMATRIX matTranslate = XMMatrixIdentity();
        XMMATRIX matScale = XMMatrixScaling(100.0f, 100.0f, 100.0f);

        matTranslate = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

        mWorld = XMMatrixTranspose(matScale * matTranslate);

    }
}
