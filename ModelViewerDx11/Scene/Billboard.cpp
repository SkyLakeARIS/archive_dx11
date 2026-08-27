#include "Billboard.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Primitive/MeshGenerator.h"
#include "../Renderer/Resources/RenderPacket.h"

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
    }

    void Billboard::Initialize(renderer::Renderer& renderer)
    {
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
    }

    void Billboard::Draw(std::vector<renderer::RenderPacket>& commandList)
    {
        const XMMATRIX matTranslate = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
        const XMMATRIX matWorld = mMatWorld * matTranslate;

        for (const auto& subMesh : mMesh.SubMeshes)
        {
            renderer::RenderPacket command = renderer::RenderPacket::MakeCommand(
                mMesh.VertexFormat,
                renderer::GetVertexStrideSize(mMesh.VertexFormat),
                renderer::eBufferUsage::Static,
                true,
                subMesh.VertexRange,
                subMesh.IndexRange,
                subMesh.Material,
                renderer::eRenderTarget::Default,
                XMMatrixTranspose(matWorld),
                renderer::eShader::Texture,
                renderer::eRasterType::Basic,
                renderer::eSamplerType::AnisotropicWrap,
                mBlendHash,
                renderer::ePrimitiveTopology::TriangleStrip,
                false,
                renderer::eDepthStencilState::DepthOffStencilOff,
                false
            );
            commandList.push_back(command);
        }
    }

    void Billboard::UpdateScaleMatrix(const XMMATRIX& viewMatrix)
    {
        // row major
        (void)memcpy(&mMatWorld.r[0].m128_f32, &viewMatrix.r[0].m128_f32, sizeof(XMFLOAT3));
        (void)memcpy(&mMatWorld.r[1].m128_f32, &viewMatrix.r[1].m128_f32, sizeof(XMFLOAT3));
        (void)memcpy(&mMatWorld.r[2].m128_f32, &viewMatrix.r[2].m128_f32, sizeof(XMFLOAT3));

        XMVECTOR determinant;
        mMatWorld = XMMatrixInverse(&determinant, mMatWorld);
    }

    void Billboard::SetTexture(HashID texHash, int16_t texSerial)
    {
        mMesh.SubMeshes.front().Material.TextureHashes[static_cast<int8_t>(renderer::eTextureType::Diffuse)] = texHash;
        mMesh.SubMeshes.front().Material.TextureSerials[static_cast<int8_t>(renderer::eTextureType::Diffuse)] = texSerial;
    }

    void Billboard::SetPosition(const XMFLOAT3& position)
    {
        mPosition = position;
    }
}
