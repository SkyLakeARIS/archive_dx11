#include "Billboard.h"
#include "Camera.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"
#include "../Renderer/Shader/ShaderManager.h"

namespace renderer
{
    struct RenderPacket;
}

namespace scene
{
    Billboard::Billboard()
        : mMesh()
        , mBlendHash(0)
        , mPosition()
        , mMatWorld(XMMatrixIdentity())
    {}

    Billboard::~Billboard()
    {
        // TODO: 추가한 MeshData 처리할 수 있는 로직이 필요함. - (종료될 떄 처리되기 때문에 당장 문제는 없음)
    }

    void Billboard::Initialize(renderer::Renderer& renderer)
    {
        // TODO: improve - 엄밀히 Material 정보에 포함되어야 할 것 같다. 
        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        renderer.CreateBlendState(blendDesc, mBlendHash);

        renderer::MeshGenerator::CreatePlane(mMesh);

        // TODO: improve - Generator로 생성하는 경우에는 Material 을 어디서 설정해 줄지? 이런 동적 생성 Mesh는 Material을 뭘로 설정할지?
    }

    void Billboard::Draw(std::vector<renderer::RenderPacket>& commandList)
    {
        renderer::RenderPacket command = {};
        command.VertexFormat = mMesh.VertexFormat;
        command.Stride = GetVertexStrideSize(mMesh.VertexFormat);
        command.bUseDynamicBuffer = false;
        command.bTransparency = true;
        command.RenderTargetType= renderer::eRenderTarget::Default;
        command.RenderState.ShaderType = renderer::eShader::RenderToTexture;
        renderer::ShaderManager::GetMaterialCbBindingDesc(command.RenderState.ShaderType, command.RenderState.CbBindingDesc);
        renderer::ShaderManager::GetMaterialTextureBindSlots(command.RenderState.ShaderType, command.RenderState.TexBindingSlots);
        renderer::ShaderManager::GetMaterialSamplerBindSlot(command.RenderState.ShaderType, command.RenderState.SamplerBindingSlot);
        command.RenderState.RasterType = renderer::eRasterType::Basic;
        command.RenderState.SamplerType = renderer::eSamplerType::AnisotropicWrap;
        command.RenderState.BlendHash = mBlendHash;
        command.RenderState.TopologyType = renderer::ePrimitiveTopology::TriangleStrip;
        command.RenderState.bUseShadowMap = false;
        command.RenderState.bUseDepthStencil = false;
        command.RenderState.bClearDepthStencilBuffer = false;
        const XMMATRIX matTranslate = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
        const XMMATRIX matWorld = mMatWorld * matTranslate;
        command.MatWorld = XMMatrixTranspose(matWorld);

        for (const auto& subMesh : mMesh.SubMeshes)
        {
            command.VertexRange = subMesh.VertexRange;
            command.IndexRange = subMesh.IndexRange;
            command.Material = subMesh.Material;
            commandList.push_back(command);
        }
    }

    void Billboard::UpdateScaleMatrix(Camera& camera)
    {
        const XMMATRIX& matView = camera.GetViewMatrix();
        // row major
        (void)memcpy(&mMatWorld.r[0].m128_f32, &matView.r[0].m128_f32, sizeof(XMFLOAT3));
        (void)memcpy(&mMatWorld.r[1].m128_f32, &matView.r[1].m128_f32, sizeof(XMFLOAT3));
        (void)memcpy(&mMatWorld.r[2].m128_f32, &matView.r[2].m128_f32, sizeof(XMFLOAT3));

        XMVECTOR determinant;
        mMatWorld = XMMatrixInverse(&determinant, mMatWorld);
    }

    void Billboard::SetTexture(HashID texHash)
    {
        mMesh.SubMeshes.front().Material.TextureHashes[static_cast<int8_t>(renderer::eTextureType::Diffuse)] = texHash;
    }

    void Billboard::SetPosition(const XMFLOAT3& position)
    {
        mPosition = position;
    }
}
