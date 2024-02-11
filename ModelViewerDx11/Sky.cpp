#include "Sky.h"
#include <string>

Sky::Sky(Renderer& renderer, Camera& camera)
    : mRenderer(&renderer)
    , mCamera(&camera)
    , mVs(nullptr)
    , mPs(nullptr)
    , mVertexBuffer(nullptr)
    , mIndexBuffer(nullptr)
    , mDevice(nullptr)
    , mDeviceContext(nullptr)
    , mInputLayout(nullptr)
    , mSampler(nullptr)
{
    renderer.AddRef();

    mDevice = renderer.GetDevice();
    mDeviceContext = renderer.GetDeviceContext();
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
    SAFETY_RELEASE(mCbMatWVP);

    SAFETY_RELEASE(mVs);
    SAFETY_RELEASE(mPs);

    SAFETY_RELEASE(mVertexBuffer);
    SAFETY_RELEASE(mIndexBuffer);
    SAFETY_RELEASE(mInputLayout);

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

    result = mRenderer->CreateDdsTextureResource(L"textures/skymap.dds", DDS_FLAGS_NONE, srvDesc, &mMesh.Texture);
    if (FAILED(result))
    {
        ASSERT(false, "Skybox - fail to create texture");
        return result;
    }
    mMesh.bLightMap = false;
    mMesh.NumTexuture = 1;
    wcscpy_s(mMesh.Name, L"skybox");


    // init shaders
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    result = mRenderer->CreateVertexShaderAndInputLayout(L"VsSkybox.hlsl", inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), &mVs, &mInputLayout);
    if(FAILED(result))
    {
        ASSERT(false, "Skybox - fail to create vertex shader and input layout");
        return result;
    }

    result = mRenderer->CreatePixelShader(L"PsSkybox.hlsl", &mPs);
    if (FAILED(result))
    {
        ASSERT(false, "Skybox - fail to create pixel shader");
        return result;
    }

    // init constant buffer

    D3D11_BUFFER_DESC cbBufferDesc;
    ZeroMemory(&cbBufferDesc, sizeof(cbBufferDesc));

    cbBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    cbBufferDesc.ByteWidth = sizeof(XMMATRIX);
    cbBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbBufferDesc.CPUAccessFlags = 0;
    cbBufferDesc.MiscFlags = 0;

    result = mDevice->CreateBuffer(&cbBufferDesc, nullptr, &mCbMatWVP);
    if (FAILED(result))
    {
        ASSERT(false, "skybox - fail to create constant buffer");
        return result;
    }

    // sampler
    D3D11_SAMPLER_DESC samplerDesc = {};
    // D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR  D3D11_FILTER_MIN_MAG_MIP_POINT
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; // 아직 정확히는 잘 모름.
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = mDevice->CreateSamplerState(&samplerDesc, &mSampler);
    if (FAILED(result))
    {
        ASSERT(false, "SamplerState 생성 실패");
        return E_FAIL;
    }

    return S_OK;
}

void Sky::Draw()
{
    // update
    XMFLOAT3 cameraPosition = mCamera->GetCameraPositionFloat();
    XMMATRIX matTranslate = XMMatrixIdentity();
    XMMATRIX matScale = XMMatrixScaling(100.0f, 100.0f, 100.0f);

    matTranslate = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);

    mWorld = matScale * matTranslate;

    // render
    uint32 stride = sizeof(Vertex);
    uint32 offset = 0;
    mDeviceContext->IASetInputLayout(mInputLayout);
    mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    mRenderer->SetRasterState(Renderer::eRasterType::Skybox);

    mDeviceContext->VSSetShader(mVs, nullptr, 0);
    mDeviceContext->PSSetShader(mPs, nullptr, 0);

    mDeviceContext->PSSetSamplers(0, 1, &mSampler);

    CbWVP cbWVP;
    cbWVP.WVP = XMMatrixTranspose(mWorld * mCamera->GetViewProjectionMatrix());
    mDeviceContext->VSSetConstantBuffers(0, 1, &mCbMatWVP);
    mDeviceContext->UpdateSubresource(mCbMatWVP, 0, nullptr, &cbWVP, 0, 0);

    mRenderer->SetDepthStencilState(true);

    mDeviceContext->PSSetShaderResources(0, 1, &mMesh.Texture);

    mDeviceContext->DrawIndexed(mMesh.IndexList.size(), 0, 0);

    mRenderer->SetDepthStencilState(false);
}

/*
 * https://www.braynzarsoft.net/viewtutorial/q16390-20-cube-mapping-skybox
 * 나중에 동적으로 생성하는 sphere 클래스를 만들어두면 좋을 것 같으므로 최대한 건들지 않는 것으로 한다.
 */
HRESULT Sky::createSphere(uint32 latLines, uint32 lonLines)
{

    const uint32 numVertex = ((latLines - 2) * lonLines) + 2;
    const uint32 numFace = ((latLines - 3) * (lonLines) * 2) + (lonLines * 2);

    float sphereYaw = 0.0f;
    float spherePitch = 0.0f;

    std::vector<Vertex> vertices(numVertex);

    vertices[0].Position.x = 0.0f;
    vertices[0].Position.y = 0.0f;
    vertices[0].Position.z = 1.0f;

    XMMATRIX matRotationX;
    XMMATRIX matRotationY;
    XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    for (uint32 i = 0; i < latLines - 2; ++i)
    {
        spherePitch = (i + 1) * (3.14 / (latLines - 1));
        matRotationX = XMMatrixRotationX(spherePitch);
        for (uint32 j = 0; j < lonLines; ++j)
        {
            sphereYaw = j * (6.28 / (lonLines));
            matRotationY = XMMatrixRotationZ(sphereYaw);
            currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (matRotationX * matRotationY));
            currVertPos = XMVector3Normalize(currVertPos);
            vertices[i * lonLines + j + 1].Position.x = XMVectorGetX(currVertPos);
            vertices[i * lonLines + j + 1].Position.y = XMVectorGetY(currVertPos);
            vertices[i * lonLines + j + 1].Position.z = XMVectorGetZ(currVertPos);
        }
    }

    vertices[numVertex - 1].Position.x = 0.0f;
    vertices[numVertex - 1].Position.y = 0.0f;
    vertices[numVertex - 1].Position.z = -1.0f;


    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * numVertex;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA bufferData;
    ZeroMemory(&bufferData, sizeof(bufferData));

    bufferData.pSysMem = vertices.data();

    HRESULT result = mDevice->CreateBuffer(&vertexBufferDesc, &bufferData, &mVertexBuffer);
    if(FAILED(result))
    {
        ASSERT(false, "skybox - fail to create vertex buffer");
        return result;
    }

    std::vector<uint32> indices(numFace * 3);
    
    int k = 0;
    for (uint32 l = 0; l < lonLines - 1; ++l)
    {
        indices[k] = 0;
        indices[k + 1] = l + 1;
        indices[k + 2] = l + 2;
        k += 3;
    }

    indices[k] = 0;
    indices[k + 1] = lonLines;
    indices[k + 2] = 1;
    k += 3;

    for (uint32 i = 0; i < latLines - 3; ++i)
    {
        for (uint32 j = 0; j < lonLines - 1; ++j)
        {
            indices[k] = i * lonLines + j + 1;
            indices[k + 1] = i * lonLines + j + 2;
            indices[k + 2] = (i + 1) * lonLines + j + 1;

            indices[k + 3] = (i + 1) * lonLines + j + 1;
            indices[k + 4] = i * lonLines + j + 2;
            indices[k + 5] = (i + 1) * lonLines + j + 2;

            k += 6; // next quad
        }

        indices[k] = (i * lonLines) + lonLines;
        indices[k + 1] = (i * lonLines) + 1;
        indices[k + 2] = ((i + 1) * lonLines) + lonLines;

        indices[k + 3] = ((i + 1) * lonLines) + lonLines;
        indices[k + 4] = (i * lonLines) + 1;
        indices[k + 5] = ((i + 1) * lonLines) + 1;

        k += 6;
    }

    for (uint32 l = 0; l < lonLines - 1; ++l)
    {
        indices[k] = numVertex - 1;
        indices[k + 1] = (numVertex - 1) - (l + 1);
        indices[k + 2] = (numVertex - 1) - (l + 2);
        k += 3;
    }

    indices[k] = numVertex - 1;
    indices[k + 1] = (numVertex - 1) - lonLines;
    indices[k + 2] = numVertex - 2;

    vertexBufferDesc.ByteWidth = sizeof(uint32) * indices.size();
    vertexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

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