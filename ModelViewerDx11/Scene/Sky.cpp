#include "Sky.h"
#include "Camera.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"
#include "../Renderer/Resources/TextureManager.h"

namespace scene
{
    Sky::Sky(Camera& camera)
        : mCamera(&camera)
        , mMesh()
        , mWorld(XMMatrixIdentity())
    {
    }

    Sky::~Sky()
    {
        mCamera = nullptr;
        // TODO: 추가한 BufferData 처리할 수 있는 로직이 필요함. - (종료될 떄 처리되기 때문에 당장 문제는 없음)
    }

    HRESULT Sky::Initialize(uint32 latLines, uint32 lonLines, renderer::TextureManager* const texManager)
    {
        renderer::MeshGenerator::CreateSphere(latLines, lonLines, mMesh);



        const int8_t* const filePath = reinterpret_cast<const int8_t*>("./AssetData/textures/skybox.dds");
        texManager->AddTextureDDS(filePath, mMesh.SubMeshes.front().Material.TextureHashes[static_cast<int8_t>(renderer::eTextureType::Diffuse)]);
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
                true,
                false
            );

            commandList.push_back(command);
        }
    }

    void Sky::Update()
    {
        // update
        XMFLOAT3 cameraPosition = mCamera->GetCameraPositionFloat();
        XMMATRIX matTranslate = XMMatrixIdentity();
        XMMATRIX matScale = XMMatrixScaling(100.0f, 100.0f, 100.0f);

        matTranslate = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

        mWorld = XMMatrixTranspose(matScale * matTranslate);

    }
}
