#include "Model.h"
#include "ModelImporter.h"

Model::Model(Renderer* renderer, Camera* camera)
    : mNumMesh(0)
    , mNumVertex(0)
    , mRenderer(renderer)
    , mCamera(camera)
{
    ASSERT(renderer != nullptr, "do not pass nullptr");
    ASSERT(camera != nullptr, "do not pass nullptr");

    mRenderer->AddRef();

    mDevice = mRenderer->GetDevice();
    ASSERT(mDevice != nullptr, "renderer was not initialized ");

    mDeviceContext = mRenderer->GetDeviceContext();
    ASSERT(mDeviceContext != nullptr, "renderer was not initialized");

    mMatWorld = XMMatrixIdentity();
}

Model::~Model()
{
    for(Mesh& mesh : mMeshes)
    {
        SAFETY_RELEASE(mesh.Texture);
    }

    SAFETY_RELEASE(mVertexBuffers);
    SAFETY_RELEASE(mIndexBuffers);

    SAFETY_RELEASE(mCbOutlineShader);
    SAFETY_RELEASE(mCbOutlineProperty);
    SAFETY_RELEASE(mCbBasicShader);

    SAFETY_RELEASE(mVertexShader);
    SAFETY_RELEASE(mPixelShader);

    SAFETY_RELEASE(mVertexShaderOutline);
    SAFETY_RELEASE(mPixelShaderOutline);

    SAFETY_RELEASE(mInputLayout);

    SAFETY_RELEASE(mSamplerState);

    mDeviceContext->Release();
    mDeviceContext = nullptr;
    mDevice->Release();
    mDevice = nullptr;


    mRenderer->Release();
    mRenderer = nullptr;

    delete[] mVertices;
    delete[] mIndices;

}

void Model::Draw()
{
    size_t vertexOffset = 0;
    size_t indexOffset = 0;

    mDeviceContext->IASetInputLayout(mInputLayout);

    const uint32 stride = sizeof(Vertex);
    const uint32 offset = 0;
    mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffers, &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffers, DXGI_FORMAT_R32_UINT, 0);

    // outline
    if(mbHighlight)
    {
        Renderer::GetInstance()->SetRasterState(Renderer::eRasterType::Outline);


        mDeviceContext->VSSetShader(mVertexShaderOutline, nullptr, 0);
        mDeviceContext->PSSetShader(mPixelShaderOutline, nullptr, 0);

        mDeviceContext->VSSetConstantBuffers(0, 1, &mCbOutlineShader);
        CbOutline cbOutline;
        cbOutline.WVP = XMMatrixTranspose(mMatWorld * mCamera->GetViewProjectionMatrix());

        mDeviceContext->UpdateSubresource(mCbOutlineShader, 0, nullptr, &cbOutline, 0, 0);
        for (size_t index = 0; index < mNumMesh; ++index)
        {
            mDeviceContext->DrawIndexed(mMeshes[index].IndexList.size(), indexOffset, vertexOffset);

            vertexOffset += mMeshes[index].Vertex.size();
            indexOffset += mMeshes[index].IndexList.size();
        }

        // reset for basic draw
        Renderer::GetInstance()->ClearDepthBuffer();
        vertexOffset = 0;
        indexOffset = 0;
    }

    Renderer::GetInstance()->SetRasterState(Renderer::eRasterType::Basic);


    mDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
    mDeviceContext->VSSetConstantBuffers(0, 1, &mCbBasicShader);

    mDeviceContext->PSSetShader(mPixelShader, nullptr, 0);
    mDeviceContext->PSSetSamplers(0, 1, &mSamplerState);


    CbBasic cbMatrices;
    cbMatrices.World = XMMatrixTranspose(mMatWorld);
    cbMatrices.WVP = XMMatrixTranspose(mMatWorld * mCamera->GetViewProjectionMatrix());

    mDeviceContext->UpdateSubresource(mCbBasicShader, 0, nullptr, &cbMatrices, 0, 0);
    // Draw
    // 위에 따라서 변경 필요. 아니면 원래 방법대로 VertexBuffer를 하나로 뭉쳐야 함.

    for (size_t index = 0; index < mNumMesh; ++index)
    {

        mDeviceContext->PSSetShaderResources(0, 1, &mMeshes[index].Texture);

        //deviceContext->Draw(mMeshes[index].Vertex.size(), offset);
        mDeviceContext->DrawIndexed(mMeshes[index].IndexList.size(), indexOffset, vertexOffset);

        vertexOffset += mMeshes[index].Vertex.size();
        indexOffset += mMeshes[index].IndexList.size();
    }
}

HRESULT Model::SetupMesh(ModelImporter& importer)
{
    // vertex / index list 데이터 합치기 용
    const uint32 sumVertexCount = importer.GetSumVertexCount();
    const uint32 sumIndexCount = importer.GetSumIndexCount();

    // 하나로 뭉침
    mVertices = new Vertex[sumVertexCount];
    mIndices = new uint32[sumIndexCount];

    // mesh - vertices, indices, textures ...
    mMeshes.swap(*importer.GetMesh());

    mNumMesh = mMeshes.size();
    ASSERT(mNumMesh > 0, "Model::SetupMesh 메시 수는 1 이상이어야 합니다. num of mesh must be over 0.");

    Vertex* posInVertices = mVertices;
    uint32* posInIndices = mIndices;
    uint32 offset = 0;
    for(size_t meshIndex = 0; meshIndex < mNumMesh; ++meshIndex)
    {
        memcpy_s(posInVertices, sizeof(Vertex) * sumVertexCount, mMeshes[meshIndex].Vertex.data(), sizeof(Vertex)*mMeshes[meshIndex].Vertex.size());
        posInVertices += mMeshes[meshIndex].Vertex.size();

        memcpy_s(posInIndices, sizeof(uint32) * sumIndexCount, mMeshes[meshIndex].IndexList.data(), sizeof(uint32)*mMeshes[meshIndex].IndexList.size());
        posInIndices += mMeshes[meshIndex].IndexList.size();

        // model importer로 이동하는게 좋을 것 같다.
        XMFLOAT3 minHeight = XMFLOAT3(100000.0f, 100000.0f, 100000.0f);
        XMFLOAT3 maxHeight = XMFLOAT3(0.0f, 0.0f, 0.0f);
        for (size_t vertexIndex = 0; vertexIndex < mMeshes[meshIndex].Vertex.size(); ++vertexIndex)
        {

            if(maxHeight.y < mMeshes[meshIndex].Vertex[vertexIndex].Position.y)
            {
                maxHeight = mMeshes[meshIndex].Vertex[vertexIndex].Position;
            }
            if(minHeight.y > mMeshes[meshIndex].Vertex[vertexIndex].Position.y)
            {
                minHeight = mMeshes[meshIndex].Vertex[vertexIndex].Position;
            }
        }


        // calc center of object

        // 센터 값이 바닥쪽에 있음.
        // 어떻게 보면 아래 방식이 뭔가 모델링 툴의 진짜 원점같아 보이긴 한다. 그래서 일단 남겨놓을 예정.
        XMVECTOR min= XMLoadFloat3(&minHeight);
        XMVECTOR max = XMLoadFloat3(&maxHeight);
        max += min;
        XMStoreFloat3(&mCenterPosition, (max / 2.0f));

        // 물체의 정중앙을 초점으로 삼고 싶기 때문에 변경
        //mCenterPosition = importer.GetModelCenter();
    }

    // and make d3d buffer

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    //desc.MiscFlags = 0;
    // vertex buffer
    D3D11_SUBRESOURCE_DATA subData;
    HRESULT result;
   // for(uint32 i = 0; i < mNumMesh; ++i)
    //{
        desc.ByteWidth = sizeof(Vertex)*sumVertexCount;
        subData.pSysMem = mVertices;

        result = mDevice->CreateBuffer(&desc, &subData, &mVertexBuffers);
        if (FAILED(result))
        {
            ASSERT(false, "버텍스 버퍼 생성 실패 failed to create VertexBuffers");
            return E_FAIL;
        }
   // }

    // index buffer
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.ByteWidth = sizeof(uint32)*sumIndexCount;
    subData.pSysMem = mIndices;
    result = mDevice->CreateBuffer(&desc, &subData, &mIndexBuffers);
    if (FAILED(result))
    {
        ASSERT(false, "인덱스 버퍼 생성 실패 failed to create IndexBuffers");
        return E_FAIL;
    }


    // constant buffers
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(CbBasic);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;

    result = mDevice->CreateBuffer(&desc, nullptr, &mCbBasicShader);
    if (FAILED(result))
    {
        ASSERT(false, "CbBasic상수 버퍼 생성 실패 failed to create CbBasic");
        return E_FAIL;
    }

    desc.ByteWidth = sizeof(CbOutline);
    result = mDevice->CreateBuffer(&desc, nullptr, &mCbOutlineShader);
    if (FAILED(result))
    {
        ASSERT(false, "CbOutline상수 버퍼 생성 실패  failed to create CbOutline");
        return E_FAIL;
    }

    desc.ByteWidth = sizeof(CbOutlineProperty);
    result = mDevice->CreateBuffer(&desc, nullptr, &mCbOutlineProperty);
    if (FAILED(result))
    {
        ASSERT(false, "CbOutlineWidth상수 버퍼 생성 실패  failed to create CbOutlineWidth");
        return E_FAIL;
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

    result = mDevice->CreateSamplerState(&samplerDesc, &mSamplerState);
    if (FAILED(result))
    {
        ASSERT(false, "SamplerState 생성 실패");
        return E_FAIL;
    }

    return result;
}

HRESULT Model::SetupShader(
    eShader shaderType,
    ID3D11VertexShader* tempVsShaderToSet,
    ID3D11PixelShader* tempPsShaderToSet,
    ID3D11InputLayout* tempInputLayout)
{
    mVertexShader = tempVsShaderToSet;
    mPixelShader = tempPsShaderToSet;
    mInputLayout = tempInputLayout;

    mVertexShader->AddRef();
    mPixelShader->AddRef();
    mInputLayout->AddRef();

    return S_OK;
}

HRESULT Model::SetupShaderFromRenderer()
{
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }

    };
    //  UINT numElements = ARRAYSIZE(layout);

    HRESULT result = Renderer::GetInstance()->CreateVertexShaderAndInputLayout(L"Shaders/VsBasic.hlsl", layout, ARRAYSIZE(layout), &mVertexShader, &mInputLayout);
    if (FAILED(result))
    {
        return result;
    }

    result = Renderer::GetInstance()->CreatePixelShader(L"Shaders/PsBasic.hlsl", &mPixelShader);
    if (FAILED(result))
    {
        return result;
    }

    // for outline
    result = Renderer::GetInstance()->CreateVertexShader(L"Shaders/VsOutline.hlsl", &mVertexShaderOutline);
    if (FAILED(result))
    {
        return result;
    }

    result = Renderer::GetInstance()->CreatePixelShader(L"Shaders/PsOutline.hlsl", &mPixelShaderOutline);
    if (FAILED(result))
    {
        return result;
    }


    return result;
}

void Model::SetHighlight(bool bSelection)
{
    mbHighlight = bSelection;
}

//
//void Model::UpdateVertexBuffer(Vertex* buffer, size_t bufferSize, size_t startIndex)
//{
//    ASSERT(buffer != nullptr, "버퍼는 nullptr가 아니어야 합니다.");
//
//    const size_t numMesh = mMeshes.size();
//    size_t offset = 0;
//    for (size_t meshIndex = 0; meshIndex < numMesh; ++meshIndex)
//    {
//        const size_t numVertex = mMeshes[meshIndex].Vertex.size();
//        for (size_t vertexIndex = 0; vertexIndex < numVertex; ++vertexIndex)
//        {
//            const size_t bufferIndex = startIndex + offset + vertexIndex;
//            buffer[bufferIndex].Position = mMeshes[meshIndex].Vertex[vertexIndex].Position;
//            buffer[bufferIndex].Normal = mMeshes[meshIndex].Vertex[vertexIndex].Normal;
//            buffer[bufferIndex].TexCoord = mMeshes[meshIndex].Vertex[vertexIndex].TexCoord;
//        }
//        offset += numVertex;
//    }
//    
//}
//
//void Model::UpdateIndexBuffer(unsigned int* buffer, size_t bufferSize, size_t startIndex)
//{
//    ASSERT(buffer != nullptr, "버퍼는 nullptr가 아니어야 합니다.");
//
//    const size_t numMesh = mMeshes.size();
//    size_t offset = 0;
//    for (size_t meshIndex = 0; meshIndex < numMesh; ++meshIndex)
//    {
//        const size_t numIndex = mMeshes[meshIndex].IndexList.size();
//        for (size_t index = 0; index < numIndex; ++index)
//        {
//            const size_t bufferIndex = startIndex + offset + index;
//            buffer[bufferIndex] = mMeshes[meshIndex].IndexList[index];
//        }
//        offset += numIndex;
//    }
//
//}

size_t Model::GetMeshCount() const
{
    return mMeshes.size();

}

const WCHAR* Model::GetMeshName(size_t meshIndex) const
{
    ASSERT(meshIndex < mMeshes.size(), "인덱스 범위를 벗어났습니다.");

    return mMeshes[meshIndex].Name;
}

size_t Model::GetVertexCount(size_t meshIndex) const
{
    ASSERT(meshIndex < mMeshes[meshIndex].Vertex.size(), "인덱스 범위를 벗어났습니다.");
    return mMeshes[meshIndex].Vertex.size();
}

size_t Model::GetIndexListCount(size_t meshIndex) const
{
    ASSERT(meshIndex < mMeshes[meshIndex].Vertex.size(), "인덱스 범위를 벗어났습니다.");
    return mMeshes[meshIndex].IndexList.size();
}

XMFLOAT3 Model::GetCenterPoint() const
{
    return mCenterPosition;
}

void Model::prepare()
{
    // 아래 두 인자에 대해서는 아직 제대로 이해하지 못함. 여러개를 전달하는건 삽질과 검색이 필요할 듯.
    //const uint32 stride = sizeof(Vertex); // 대개 Vertex가 서로 다른 stride()가질 때 사용하는 듯, 예로 Pos+nor+tex 인 vertex 하나 pos+tex인 vertex하나
    //const uint32 offset = 0;
    //mDeviceContext->IASetVertexBuffers(0, mNumMesh, mVertexBuffers, &stride, &offset);
    //mDeviceContext->IASetIndexBuffer(gIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

}

