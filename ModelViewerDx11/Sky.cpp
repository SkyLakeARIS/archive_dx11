#include "Sky.h"
#include <string>

Sky::Sky(Renderer& renderer, Camera& camera)
    : mRenderer(&renderer)
    , mCamera(&camera)
    , mDevice(nullptr)
    , mDeviceContext(nullptr)
    , mWorld(XMMatrixIdentity())
    , mVertexBuffer(nullptr)
    , mIndexBuffer(nullptr)
    , mCbMatWorld(nullptr)
    , mSampler(nullptr)
    , mLatLines(0)
    , mLonLines(0)
{
    renderer.AddRef();

    mDevice        = renderer.GetDevice();
    mDeviceContext = renderer.GetDeviceContext();

    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(Renderer::CbWorld);
    HRESULT result = Renderer::GetInstance()->CreateConstantBuffer(desc, &mCbMatWorld);
    ASSERT(result == S_OK, "mCbMatWorld ���� ����.");

}

Sky::~Sky()
{
    mRenderer->Release();
    mRenderer = nullptr;

    mCamera = nullptr;

    mDevice->Release();
    mDevice = nullptr;

    mDeviceContext->Release();
    mDeviceContext = nullptr;


    SAFETY_RELEASE(mSampler);
    SAFETY_RELEASE(mCbMatWorld);

    SAFETY_RELEASE(mVertexBuffer);
    SAFETY_RELEASE(mIndexBuffer);

    SAFETY_RELEASE(mMesh.Texture);

}

HRESULT Sky::Initialize(uint32 latLines, uint32 lonLines)
{

    // init mesh info
    HRESULT result;
    result = createSphere(latLines, lonLines);
    if(FAILED(result))
    {
        return result;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 1;
    srvDesc.TextureCube.MostDetailedMip = 0;
    
    result = mRenderer->CreateDdsTextureResource(L"textures/skybox.dds", DDS_FLAGS_NONE, srvDesc, &mMesh.Texture);
    if (FAILED(result))
    {
        ASSERT(false, "Skybox - fail to create texture");
        return result;
    }
    mMesh.bLightMap = false;
    mMesh.NumTexuture = 1;
    wcscpy_s(mMesh.Name, L"skybox");


    // sampler
    D3D11_SAMPLER_DESC samplerDesc = {};
    // D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR  D3D11_FILTER_MIN_MAG_MIP_POINT
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; // ���� ��Ȯ���� �� ��.
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = mDevice->CreateSamplerState(&samplerDesc, &mSampler);
    if (FAILED(result))
    {
        ASSERT(false, "SamplerState ���� ����");
        return E_FAIL;
    }

    return S_OK;
}

void Sky::Draw()
{
    // render
    mRenderer->SetInputLayoutTo(Renderer::eInputLayout::PT);

    uint32 stride = sizeof(Vertex);
    uint32 offset = 0;
    mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    mRenderer->SetRasterState(Renderer::eRasterType::Skybox);

    mRenderer->SetShaderTo(Renderer::eShader::Skybox);

    mDeviceContext->PSSetSamplers(0, 1, &mSampler);

   

    Renderer::GetInstance()->BindCbToVsByObj(0U, 1U, &mCbMatWorld);
    Renderer::GetInstance()->BindCbToVsByType(1U, 1U, Renderer::eCbType::CbViewProj);

    mRenderer->SetDepthStencilState(true);

    mDeviceContext->PSSetShaderResources(0, 1, &mMesh.Texture);

    mDeviceContext->DrawIndexed(static_cast<uint32_t>(mMesh.IndexList.size()), 0, 0);

    mRenderer->SetDepthStencilState(false);

}

void Sky::Update()
{
    // update
    XMFLOAT3 cameraPosition = mCamera->GetCameraPositionFloat();
    XMMATRIX matTranslate = XMMatrixIdentity();
    XMMATRIX matScale = XMMatrixScaling(100.0f, 100.0f, 100.0f);

    matTranslate = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

    mWorld = matScale * matTranslate;

    Renderer::CbWorld cbWVP;
    cbWVP.Matrix = XMMatrixTranspose(mWorld);
    Renderer::GetInstance()->UpdateCbTo(mCbMatWorld, &cbWVP);
}

/*
 * https://www.braynzarsoft.net/viewtutorial/q16390-20-cube-mapping-skybox
 * ���߿� �������� �����ϴ� sphere Ŭ������ �����θ� ���� �� �����Ƿ� �ִ��� �ǵ��� �ʴ� ������ �Ѵ�.
 */
HRESULT Sky::createSphere(uint32 latLines, uint32 lonLines)
{

    const uint32 numVertex = ((latLines - 2) * lonLines) + 2;
    const uint32 numFace = ((latLines - 3) * (lonLines) * 2) + (lonLines * 2);


    std::vector<Vertex> vertices(numVertex);

    vertices[0].Position.x = 0.0f;
    vertices[0].Position.y = 0.0f;
    vertices[0].Position.z = 1.0f;

    XMMATRIX matRotationX;
    XMMATRIX matRotationY;
    float sphereYaw = 0.0f;
    float spherePitch = 0.0f;
    XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    for (uint32 i = 0; i < latLines - 2U; ++i)
    {
        spherePitch = (float)(i + 1U) * (3.14f / (float)(latLines - 1U));
        matRotationX = XMMatrixRotationX(spherePitch);
        for (uint32 j = 0U; j < lonLines; ++j)
        {
            sphereYaw = (float)j * (6.28f / (float)lonLines);
            matRotationY = XMMatrixRotationZ(sphereYaw);
            currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (matRotationX * matRotationY));
            currVertPos = XMVector3Normalize(currVertPos);
            uint32 index = i * lonLines + j + 1U;
            vertices[index].Position.x = XMVectorGetX(currVertPos);
            vertices[index].Position.y = XMVectorGetY(currVertPos);
            vertices[index].Position.z = XMVectorGetZ(currVertPos);
        }
    }

    vertices[numVertex - 1U].Position.x = 0.0f;
    vertices[numVertex - 1U].Position.y = 0.0f;
    vertices[numVertex - 1U].Position.z = -1.0f;


    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * numVertex;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0U;
    vertexBufferDesc.MiscFlags = 0U;

    D3D11_SUBRESOURCE_DATA bufferData;
    ZeroMemory(&bufferData, sizeof(bufferData));

    bufferData.pSysMem = vertices.data();

    HRESULT result = mDevice->CreateBuffer(&vertexBufferDesc, &bufferData, &mVertexBuffer);
    if(FAILED(result))
    {
        ASSERT(false, "skybox - fail to create vertex buffer");
        return result;
    }

    std::vector<uint32> indices(numFace * 3U);
    
    int k = 0;
    for (uint32 l = 0U; l < lonLines - 1U; ++l)
    {
        indices[k] = 0U;
        indices[k + 1] = l + 1U;
        indices[k + 2] = l + 2U;
        k += 3;
    }

    indices[k] = 0U;
    indices[k + 1] = lonLines;
    indices[k + 2] = 1U;
    k += 3;

    for (uint32 i = 0U; i < latLines - 3U; ++i)
    {
        for (uint32 j = 0U; j < lonLines - 1U; ++j)
        {
            indices[k] = i * lonLines + j + 1U;
            indices[k + 1] = i * lonLines + j + 2U;
            indices[k + 2] = (i + 1U) * lonLines + j + 1U;

            indices[k + 3] = (i + 1U) * lonLines + j + 1U;
            indices[k + 4] = i * lonLines + j + 2U;
            indices[k + 5] = (i + 1U) * lonLines + j + 2U;

            k += 6; // next quad
        }

        indices[k] = (i * lonLines) + lonLines;
        indices[k + 1] = (i * lonLines) + 1U;
        indices[k + 2] = ((i + 1U) * lonLines) + lonLines;

        indices[k + 3] = ((i + 1U) * lonLines) + lonLines;
        indices[k + 4] = (i * lonLines) + 1U;
        indices[k + 5] = ((i + 1U) * lonLines) + 1U;

        k += 6;
    }

    for (uint32 l = 0U; l < lonLines - 1U; ++l)
    {
        indices[k] = numVertex - 1U;
        indices[k + 1] = (numVertex - 1U) - (l + 1U);
        indices[k + 2] = (numVertex - 1U) - (l + 2U);
        k += 3;
    }

    indices[k] = numVertex - 1U;
    indices[k + 1] = (numVertex - 1U) - lonLines;
    indices[k + 2] = numVertex - 2U;

    vertexBufferDesc.ByteWidth = sizeof(uint32) * static_cast<uint32_t>(indices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0U;
    vertexBufferDesc.MiscFlags = 0U;

    ZeroMemory(&bufferData, sizeof(bufferData));
    bufferData.pSysMem = indices.data();

    result = mDevice->CreateBuffer(&vertexBufferDesc, &bufferData, &mIndexBuffer);
    if (FAILED(result))
    {
        ASSERT(false, "skybox - fail to create vertex buffer");
        return result;
    }


    mMesh.Vertex.swap(vertices);
    mMesh.IndexList.swap(indices);
    return S_OK;
}