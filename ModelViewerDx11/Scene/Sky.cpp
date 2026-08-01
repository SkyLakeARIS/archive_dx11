#include "Sky.h"
#include "Camera.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"
#include "../Renderer/Resources/TextureManager.h"
#include "../Renderer/Shader/ShaderManager.h"

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
        texManager->AddDTextureDDS(filePath, mMesh.SubMeshes.front().Material.TextureHashes[static_cast<int8_t>(renderer::eTextureType::Diffuse)]);
        return S_OK;
    }

    void Sky::Draw(std::vector<renderer::RenderPacket>& commandList)
    {
        renderer::RenderPacket command = {};
        command.VertexFormat = mMesh.VertexFormat;
        command.Stride = GetVertexStrideSize(mMesh.VertexFormat);
        command.bUseDynamicBuffer = false;
        command.bTransparency = false;
        command.RenderTargetType = renderer::eRenderTarget::Default;
        command.MatWorld = mWorld;
        command.RenderState.ShaderType = renderer::eShader::Skybox;
        renderer::ShaderManager::GetMaterialCbBindingDesc(command.RenderState.ShaderType, command.RenderState.CbBindingDesc);
        renderer::ShaderManager::GetMaterialTextureBindSlots(command.RenderState.ShaderType, command.RenderState.TexBindingSlots);
        renderer::ShaderManager::GetMaterialSamplerBindSlot(command.RenderState.ShaderType, command.RenderState.SamplerBindingSlot);
        command.RenderState.RasterType = renderer::eRasterType::Skybox;
        command.RenderState.SamplerType = renderer::eSamplerType::AnisotropicWrap;
        command.RenderState.BlendHash = 0;
        command.RenderState.TopologyType = renderer::ePrimitiveTopology::Triangles;
        command.RenderState.bUseDepthStencil = true;
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
