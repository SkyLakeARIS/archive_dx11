#include "Light.h"

#include "Plane.h"
#include "Renderer.h"

Light::Light(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 color)
    : mPosition(pos)
    , mDirection(dir)
    , mColor(color)
    , mMatProj(XMMatrixIdentity())
    , mMatViewProj(XMMatrixIdentity())
    , mCbMatWorld(nullptr)
    , mMatWorld(XMMatrixIdentity())
{
    mMesh = new Plane();
    mMesh->SetPosition(mPosition);

    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(Renderer::CbWorld);
    HRESULT result = Renderer::GetInstance()->CreateConstantBuffer(desc, &mCbMatWorld);
    ASSERT(result == S_OK, "mCbMatWorld 생성 실패.");

    updateMatrices();
}

Light::~Light()
{

    if(mMesh)
    {
        delete mMesh;
        mMesh = nullptr;
    }

    SAFETY_RELEASE(mCbMatWorld);

    SAFETY_RELEASE(mBlendState);
}

void Light::Initialize()
{
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
 //   desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipLevels = 1;
    desc.Texture2D.MostDetailedMip = 0;
    ID3D11ShaderResourceView * texLightIcon = nullptr;
    
    HRESULT result = Renderer::GetInstance()->CreateTextureResource(L"./textures/lightIcon.png", WIC_FLAGS_NONE, desc, &texLightIcon);
    ASSERT(SUCCEEDED(result), "Light Init - failed to ready texture");
    SET_PRIVATE_DATA(texLightIcon, "TexLightIcon");

    mMesh->SetTexture(texLightIcon);

    ID3D11Device* device = Renderer::GetInstance()->GetDevice();

    // alpha blend 
    D3D11_BLEND_DESC blendDesc={};
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha= D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    device->CreateBlendState(&blendDesc, &mBlendState);

    device->Release();
    SAFETY_RELEASE(texLightIcon);
}

void Light::Update(Camera* camera)
{
    mMesh->Update();
    mMesh->GetWorldMatrix(mMatWorld);
}

void Light::Draw(double deltaTime)
{
    Renderer::GetInstance()->SetInputLayoutTo(Renderer::eInputLayout::PT);
    Renderer::GetInstance()->SetShaderTo(Renderer::eShader::RenderToTexture);

    ID3D11DeviceContext* deviceContext =  Renderer::GetInstance()->GetDeviceContext();
    Renderer::GetInstance()->SetRasterState(Renderer::eRasterType::Basic);
    deviceContext->OMSetBlendState(mBlendState, nullptr, 0xffffffff);

    deviceContext->Release();

    Renderer::CbWorld cbMatWorld;
    cbMatWorld.Matrix = XMMatrixTranspose(mMatWorld);
    Renderer::GetInstance()->UpdateCbTo(mCbMatWorld, &cbMatWorld);

    Renderer::GetInstance()->BindCbToVsByObj(0U, 1U, &mCbMatWorld);
    Renderer::GetInstance()->BindCbToVsByType(1U, 1U, Renderer::eCbType::CbViewProj);

    mMesh->Draw();
}

void Light::Move(double deltaTime, float direction)
{
    XMFLOAT3 dir = mDirection;
    XMFLOAT3 pos = mPosition;
    XMVECTOR vDir = XMLoadFloat3(&dir);
    XMVECTOR vPos = XMLoadFloat3(&pos);
    vDir *= deltaTime * 100.0;
    vPos += vDir;
    XMStoreFloat3(&mPosition, (vPos));
    mMesh->SetPosition(mPosition);

    updateMatrices();
}

void Light::SetDirection(XMFLOAT3 dir)
{
    XMFLOAT3 pos = mPosition;
    XMVECTOR vDir = XMLoadFloat3(&dir);
    XMVECTOR vPos = XMLoadFloat3(&pos);
    vDir = vDir - vPos;
    XMStoreFloat3(&mDirection, (vDir));

   // mDirection = dir;

    updateMatrices();
}

void Light::SetColor(XMFLOAT3 color)
{
    mColor = color;
}

XMFLOAT4 Light::GetDirection() const
{
    return XMFLOAT4(mDirection.x, mDirection.y, mDirection.z, 1.0f);
}

XMFLOAT4 Light::GetPosition() const
{
    return XMFLOAT4(mPosition.x, mPosition.y, mPosition.z, 1.0f);
}

XMFLOAT4 Light::GetColor() const
{
    return XMFLOAT4(mColor.x, mColor.y, mColor.z, 1.0f);
}

const XMMATRIX* const Light::GetViewProjMatrix() const
{
    return &mMatViewProj;
}

void Light::updateMatrices()
{
    mMatView = XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mDirection), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
   //mMatProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0, 69.0f, 120.0f);
    mMatProj = XMMatrixOrthographicOffCenterLH(-1.0f, 1.0f, -1.0f, 1.0f, 69.0f, 100.0f);
    mMatViewProj = mMatView * mMatProj;

    Renderer::CbLightViewProjMatrix cbLightVpMat;
    cbLightVpMat.Matrix = XMMatrixTranspose(mMatViewProj);
    //deviceContext->UpdateSubresource(mCB, 0, nullptr, &cbWvp, 0U, 0U);
    Renderer::GetInstance()->UpdateCB(Renderer::eCbType::CbLightViewProjMatrix, &cbLightVpMat);

}
