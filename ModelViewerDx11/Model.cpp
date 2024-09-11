﻿#include "Model.h"

#include "Light.h"
#include "ModelImporter.h"

Model::Model(Renderer* renderer, Camera* camera)
    : mRenderer(renderer)
    , mCamera(camera)
    , mNumMesh(0)
    , mNumVertex(0)
    , mVertices(nullptr)
    , mIndices(nullptr)
    , mVertexBuffers(nullptr)
    , mIndexBuffers(nullptr)
    , mCbBasicShader(nullptr)
    , mCbOutlineShader(nullptr)
    , mCbOutlineProperty(nullptr)
    , mCbLight(nullptr)
    , mCbMaterial(nullptr)
    , mCbCamera(nullptr)
    , mCbLightMatrix(nullptr)
    , mCenterPosition(0.0f, 0.0f, 0.0f)
    , mMatRotation(XMMatrixIdentity())
    , mMatScale(XMMatrixIdentity())
    , mLight(nullptr)
    , mSamplerState(nullptr)
    , mbHighlight(false)
    , mbActiveEmissive(false)
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
        SAFETY_RELEASE(mesh.TextureNormal);
    }


    SAFETY_RELEASE(mVertexBuffers);
    SAFETY_RELEASE(mIndexBuffers);

    SAFETY_RELEASE(mCbOutlineShader);
    SAFETY_RELEASE(mCbOutlineProperty);
    SAFETY_RELEASE(mCbBasicShader);
    SAFETY_RELEASE(mCbLight);
    SAFETY_RELEASE(mCbMaterial);
    SAFETY_RELEASE(mCbCamera);
    SAFETY_RELEASE(mCbLightMatrix);


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
    mRenderer->SetInputLayoutTo(Renderer::eInputLayout::Basic);

    // mDeviceContext->IASetInputLayout(mInputLayout);

    constexpr uint32 stride = sizeof(Vertex);
    constexpr uint32 offset = 0U;
    mDeviceContext->IASetVertexBuffers(0U, 1U, &mVertexBuffers, &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffers, DXGI_FORMAT_R32_UINT, 0U);

    // outline
    int32_t vertexOffset = 0;
    uint32_t indexOffset = 0U;
    if(mbHighlight)
    {

        Renderer::GetInstance()->SetRasterState(Renderer::eRasterType::CullBack);

        mRenderer->SetShaderTo(Renderer::eShader::Outline);
        //mDeviceContext->VSSetShader(mVertexShaderOutline, nullptr, 0U);
        //mDeviceContext->PSSetShader(mPixelShaderOutline, nullptr, 0U);

        mDeviceContext->VSSetConstantBuffers(0U, 1U, &mCbOutlineShader);
        CbOutline cbOutline;
        cbOutline.WVP = XMMatrixTranspose(mMatWorld * mCamera->GetViewProjectionMatrix());

        mDeviceContext->UpdateSubresource(mCbOutlineShader, 0, nullptr, &cbOutline, 0U, 0U);

        for (uint32_t index = 0U; index < mNumMesh; ++index)
        {
            mDeviceContext->DrawIndexed(static_cast<uint32_t>(mMeshes[index].IndexList.size()), indexOffset, vertexOffset);

            vertexOffset += static_cast<int32_t>(mMeshes[index].Vertex.size());
            indexOffset += static_cast<uint32_t>(mMeshes[index].IndexList.size());
        }

        // reset for basic draw
        Renderer::GetInstance()->ClearDepthBuffer();
    }

    Renderer::GetInstance()->SetRasterState(Renderer::eRasterType::Basic);

    mRenderer->SetShaderTo(Renderer::eShader::BasicWithShadow);

  //  mDeviceContext->VSSetShader(mVsBasicWithShadow, nullptr, 0U);
    mDeviceContext->VSSetConstantBuffers(0U, 1U, &mCbBasicShader);
    mDeviceContext->VSSetConstantBuffers(1U, 1U, &mCbLightMatrix);
    mDeviceContext->VSSetConstantBuffers(2U, 1U, &mCbLight);
    mDeviceContext->VSSetConstantBuffers(3U, 1U, &mCbCamera);

  //  mDeviceContext->PSSetShader(mPsBasicWithShadow, nullptr, 0U);
    mDeviceContext->PSSetSamplers(0U, 1U, &mSamplerState);

    mDeviceContext->PSSetConstantBuffers(0U, 1U, &mCbMaterial);

    //texShadow->Release();

    auto* texShadow = Renderer::GetInstance()->GetShadowTexture();
    mDeviceContext->PSSetShaderResources(2U, 1U, &texShadow);


    CbBasic cbMatrices;
    cbMatrices.World = XMMatrixTranspose(mMatWorld);
    cbMatrices.WVP = XMMatrixTranspose(mMatWorld * mCamera->GetViewProjectionMatrix());

    mDeviceContext->UpdateSubresource(mCbBasicShader, 0U, nullptr, &cbMatrices, 0U, 0U);

    CbLight cbLight;

    // TODO : light manager 클래스 제작필요
    cbLight.LightColor = mLight->GetColor();
    cbLight.LightDir = mLight->GetPosition();
    mDeviceContext->UpdateSubresource(mCbLight, 0U, nullptr, &cbLight, 0U, 0U);

    CbCamera cbCamera;
    cbCamera.Position = mCamera->GetCameraPositionFloat();
    mDeviceContext->UpdateSubresource(mCbCamera, 0U, nullptr, &cbCamera, 0U, 0U);

    CbLightMatrix cbLightMatrix;
    cbLightMatrix.WVP = XMMatrixTranspose(mMatWorld * (*mLight->GetViewProjMatrix()));
    mDeviceContext->UpdateSubresource(mCbLightMatrix, 0U, nullptr, &cbLightMatrix, 0U, 0U);

    // Draw
    // 위에 따라서 변경 필요. 아니면 원래 방법대로 VertexBuffer를 하나로 뭉쳐야 함.
    vertexOffset = 0;
    indexOffset = 0U;
    for (size_t index = 0U; index < mNumMesh; ++index)
    {
        // TODO ligtmap 테스트 - 별도 셰이더를 제작하는 방법도 고려
         // TODO 별도 셰이더 생성 완료 -> 셰이더 매니저 생성 -> 셰이더 기준으로 렌더링
         // 일단 셰이더 매니저를 생성하기 전까지는 이렇게 둠.
        //if (mMeshes[index].bLightMap)
        //{
        //    // for face
        //    mDeviceContext->PSSetShader(mPixelShaderLightMap, nullptr, 0U);

        //    mDeviceContext->PSSetSamplers(0U, 1U, &mSamplerState);

        //    mDeviceContext->PSSetConstantBuffers(0U, 1U, &mCbMaterial);
        //    mDeviceContext->PSSetShaderResources(2U, 1U, &texShadow);
        //    mDeviceContext->UpdateSubresource(mCbBasicShader, 0U, nullptr, &cbMatrices, 0U, 0U);
        //}

        mDeviceContext->PSSetShaderResources(0U, 1U, &mMeshes[index].Texture);
        mDeviceContext->PSSetShaderResources(1U, 1U, &mMeshes[index].TextureNormal);
        CbMaterial cbMaterial;
        ZeroMemory(&cbMaterial, sizeof(CbMaterial));

        memcpy(&cbMaterial, &mMeshes[index].Material, sizeof(Material));
        if(!mbActiveEmissive)
        {
            cbMaterial.Emissive = XMFLOAT3(0.0f, 0.0f, 0.0f);
        }
        mDeviceContext->UpdateSubresource(mCbMaterial, 0U, nullptr, &cbMaterial, 0U, 0U);

        //deviceContext->Draw(mMeshes[index].Vertex.size(), offset);
        mDeviceContext->DrawIndexed(static_cast<uint32_t>(mMeshes[index].IndexList.size()), indexOffset, vertexOffset);

        vertexOffset += static_cast<int32_t>(mMeshes[index].Vertex.size());
        indexOffset += static_cast<uint32_t>(mMeshes[index].IndexList.size());

        //// TODO 별도 셰이더 생성 완료 -> 셰이더 매니저 생성 -> 셰이더 기준으로 렌더링
        //// 일단 셰이더 매니저를 생성하기 전까지는 이렇게 둠.
        //if (mMeshes[index].bLightMap)
        //{
        //    // reset shader
        //    mDeviceContext->PSSetShader(mPixelShader, nullptr, 0U);

        //    mDeviceContext->PSSetSamplers(0U, 1U, &mSamplerState);
        //    mDeviceContext->PSSetShaderResources(0U, 1U, &mMeshes[index].Texture);

        //    mDeviceContext->PSSetConstantBuffers(0U, 1U, &mCbMaterial);

        //    mDeviceContext->UpdateSubresource(mCbBasicShader, 0U, nullptr, &cbMatrices, 0U, 0U);
        //}
    }

    ID3D11ShaderResourceView* unbind = nullptr;
    mDeviceContext->PSSetShaderResources(2U, 1U, &unbind);
}

// shadow 렌더링이 목적이므로 최대한 단순하게
void Model::DrawShadow()
{

    //mDeviceContext->IASetInputLayout(mSimpleInputLayout);
    mRenderer->SetInputLayoutTo(Renderer::eInputLayout::Simple);

    const uint32 stride = sizeof(Vertex);
    const uint32 offset = 0U;
    mDeviceContext->IASetVertexBuffers(0U, 1U, &mVertexBuffers, &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffers, DXGI_FORMAT_R32_UINT, 0U);

    Renderer::GetInstance()->SetRasterState(Renderer::eRasterType::Outline);
    mRenderer->SetShaderTo(Renderer::eShader::Shadow);

   // mDeviceContext->VSSetShader(mVsShadow, nullptr, 0U);
    mDeviceContext->VSSetConstantBuffers(0U, 1U, &mCbLightMatrix);


  //  mDeviceContext->PSSetShader(mPsShadow, nullptr, 0U);

    CbLightMatrix cbLightMatrix;
    cbLightMatrix.WVP = XMMatrixTranspose(mMatWorld * (*mLight->GetViewProjMatrix()));
    mDeviceContext->UpdateSubresource(mCbLightMatrix, 0U, nullptr, &cbLightMatrix, 0U, 0U);

    // Draw
    int32_t vertexOffset = 0;
    uint32_t indexOffset = 0U;
    for (size_t index = 0U; index < mNumMesh; ++index)
    {
        mDeviceContext->DrawIndexed(static_cast<uint32_t>(mMeshes[index].IndexList.size()), indexOffset, vertexOffset);

        vertexOffset += static_cast<int32_t>(mMeshes[index].Vertex.size());
        indexOffset += static_cast<uint32_t>(mMeshes[index].IndexList.size());
    }
}

void Model::SetLight(Light* light)
{
    mLight = light;
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
    mNumMesh = importer.GetMeshCount();
    ASSERT(mNumMesh > 0U, "Model::SetupMesh 메시 수는 1 이상이어야 합니다. num of mesh must be over 0.");

    Vertex* posInVertices = mVertices;
    uint32* posInIndices = mIndices;
    uint32 offset = 0U;
    for(size_t meshIndex = 0U; meshIndex < mNumMesh; ++meshIndex)
    {
        Mesh* mesh = importer.GetMesh(meshIndex);
        Mesh newMesh;
        newMesh.Vertex.swap(mesh->Vertex);
        newMesh.IndexList.swap(mesh->IndexList);
        newMesh.bLightMap = mesh->bLightMap;
        memcpy(newMesh.Name, mesh->Name, sizeof(WCHAR) * MESH_NAME_LENGTH);
        newMesh.Texture = mesh->Texture;
        mesh->Texture = nullptr;
        newMesh.TextureNormal = mesh->TextureNormal;
        mesh->TextureNormal = nullptr;
        newMesh.NumTexuture = mesh->NumTexuture;
        memcpy(&newMesh.Material, &mesh->Material, sizeof(Material));

        mMeshes.push_back(newMesh);

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
    desc.CPUAccessFlags = 0U;
    //desc.MiscFlags = 0;
    // vertex buffer
    D3D11_SUBRESOURCE_DATA subData;
    HRESULT result;

    desc.ByteWidth = sizeof(Vertex)*sumVertexCount;
    subData.pSysMem = mVertices;

    result = mDevice->CreateBuffer(&desc, &subData, &mVertexBuffers);
    if (FAILED(result))
    {
        ASSERT(false, "버텍스 버퍼 생성 실패 failed to create VertexBuffers");
        return E_FAIL;
    }


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

    desc.ByteWidth = sizeof(CbLight);
    result = mDevice->CreateBuffer(&desc, nullptr, &mCbLight);
    if (FAILED(result))
    {
        ASSERT(false, "CbLight상수 버퍼 생성 실패  failed to create CbLight");
        return E_FAIL;
    }

    desc.ByteWidth = sizeof(CbMaterial);
    result = mDevice->CreateBuffer(&desc, nullptr, &mCbMaterial);
    if (FAILED(result))
    {
        ASSERT(false, "mCbMaterial상수 버퍼 생성 실패  failed to create mCbMaterial");
        return E_FAIL;
    }

    desc.ByteWidth = sizeof(CbCamera);
    result = mDevice->CreateBuffer(&desc, nullptr, &mCbCamera);
    if (FAILED(result))
    {
        ASSERT(false, "mCbCamera상수 버퍼 생성 실패  failed to create mCbCamera");
        return E_FAIL;
    }


    desc.ByteWidth = sizeof(CbLightMatrix);
    result = mDevice->CreateBuffer(&desc, nullptr, &mCbLightMatrix);
    if (FAILED(result))
    {
        ASSERT(false, "mCbLightMatrix상수 버퍼 생성 실패  failed to create mCbLightMatrix");
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

HRESULT Model::SetupShaderFromRenderer()
{
    return S_OK;
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
    XMFLOAT3 pos = mCenterPosition;
    pos.y += 1.0f;
    return pos;
}

void Model::prepare()
{
    // 아래 두 인자에 대해서는 아직 제대로 이해하지 못함. 여러개를 전달하는건 삽질과 검색이 필요할 듯.
    //const uint32 stride = sizeof(Vertex); // 대개 Vertex가 서로 다른 stride()가질 때 사용하는 듯, 예로 Pos+nor+tex 인 vertex 하나 pos+tex인 vertex하나
    //const uint32 offset = 0;
    //mDeviceContext->IASetVertexBuffers(0, mNumMesh, mVertexBuffers, &stride, &offset);
    //mDeviceContext->IASetIndexBuffer(gIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

}

