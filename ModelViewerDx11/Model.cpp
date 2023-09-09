#include "Model.h"
#include "ModelImporter.h"

Model::Model(Renderer* renderer, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
    : mNumMesh(0)
    , mNumVertex(0)
    , mRenderer(renderer)
    , mDevice(device)
    , mDeviceContext(deviceContext)
{
    mDevice->AddRef();
    mDeviceContext->AddRef();
}

Model::~Model()
{
    for(Mesh& mesh : mMeshes)
    {
        SAFETY_RELEASE(mesh.Texture);
    }

    SAFETY_RELEASE(mVertexBuffers);

    SAFETY_RELEASE(mIndexBuffers);

    mDevice->Release();
    mDevice = nullptr;
    mDeviceContext->Release();
    mDeviceContext = nullptr;

    delete[] vertices;
    delete[] indices;
}

void Model::Draw()
{
    // 현재는 비효율적 같지만 다른 오브젝트도 그리기 시작하면 해야하지 않을까 생각 중.
        // 근데 buffer를 여러개로 나누면 draw는 어떻게 ..?
        // => DrawIndexedInstanced 조사
    //prepare();
    mDeviceContext->IASetInputLayout(mInputLayout);
    const uint32 stride = sizeof(Vertex);
    const uint32 offset = 0;

    mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffers, &stride, &offset);
    mDeviceContext->IASetIndexBuffer(mIndexBuffers, DXGI_FORMAT_R32_UINT, 0);

   mDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
    mDeviceContext->PSSetShader(mPixelShader, nullptr, 0);



    // Draw
    // 위에 따라서 변경 필요. 아니면 원래 방법대로 VertexBuffer를 하나로 뭉쳐야 함.
    size_t vertexOffset = 0;
    size_t indexOffset = 0;
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
    vertices = new Vertex[sumVertexCount];
    indices = new uint32[sumIndexCount];

    // mesh - vertices, indices, textures ...
    mMeshes.swap(*importer.GetMesh());

    mNumMesh = mMeshes.size();
    ASSERT(mNumMesh > 0, "Model::SetupMesh num of mesh is 0.");

    Vertex* posInVertices = vertices;
    uint32* posInIndices = indices;
    for(size_t meshIndex = 0; meshIndex < mNumMesh; ++meshIndex)
    {
      //  mMeshes.push_back(Mesh());
     //   Mesh& mesh = mMeshes.back();


        memcpy_s(posInVertices, sizeof(Vertex) * sumVertexCount, mMeshes[meshIndex].Vertex.data(), sizeof(Vertex)*mMeshes[meshIndex].Vertex.size());
        posInVertices += mMeshes[meshIndex].Vertex.size();

        memcpy_s(posInIndices, sizeof(uint32) * sumIndexCount, mMeshes[meshIndex].IndexList.data(), sizeof(uint32)*mMeshes[meshIndex].IndexList.size());
        posInIndices += mMeshes[meshIndex].IndexList.size();

        // vertex info
     //   const uint16 numVertex = meshes[meshIndex].Vertex.size(); // ..?
      //  mMeshes[meshIndex].Vertex.reserve(numVertex);
        XMFLOAT3 minHeight = XMFLOAT3(100000.0f, 100000.0f, 100000.0f);
        XMFLOAT3 maxHeight = XMFLOAT3(0.0f, 0.0f, 0.0f);
        for (size_t vertexIndex = 0; vertexIndex < mMeshes[meshIndex].Vertex.size(); ++vertexIndex)
        {
            //// 괜히 일을 두번하는 느낌. vector가 push, emplace시에 복사로 동작하는지 확인해보기 
            //mesh.Vertex.push_back(Vertex());
            //Vertex& vertex = mesh.Vertex.back();

            //vertex.Position = meshes[meshIndex].Vertex[vertexIndex].Position;
            //vertex.Normal = meshes[meshIndex].Vertex[vertexIndex].Normal;
            //vertex.TexCoord = meshes[meshIndex].Vertex[vertexIndex].TexCoord;
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
        XMVECTOR min= XMLoadFloat3(&minHeight);
        XMVECTOR max = XMLoadFloat3(&maxHeight);
        max += min;
        XMStoreFloat3(&mCenterPosition, (max / 2.0f));

        // indexList info
        //const int numIndexList = meshes[meshIndex].IndexList.size();
        //mMeshes[meshIndex].IndexList.reserve(numIndexList);
        //for (size_t index = 0; index < numIndexList; ++index)
        //{
        //    mesh.IndexList.push_back(meshes[meshIndex].IndexList[index]);
        //}

        // mesh name
       // wcscpy_s(mesh.Name, MESH_NAME_LENGTH, meshes[meshIndex].Name);
        //mesh.Name[MESH_NAME_LENGTH - 1] = (WCHAR)L"\n";

        // texture info
       // mesh.Texture = meshes[meshIndex].Texture;
       // mesh.NumTexuture = meshes[meshIndex].NumTexuture;
       // mesh.HasTexture = meshes[meshIndex].HasTexture;
        
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
        subData.pSysMem = vertices;

        result = mDevice->CreateBuffer(&desc, &subData, &mVertexBuffers);
        if (FAILED(result))
        {
            ASSERT(false, "버텍스 버퍼 생성 실패");
            return E_FAIL;
        }
   // }

    // index buffer
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

   // for(uint32 i = 0; i < mNumMesh; ++i)
   // {
        desc.ByteWidth = sizeof(uint32)*sumIndexCount;
        subData.pSysMem = indices;
        result = mDevice->CreateBuffer(&desc, &subData, &mIndexBuffers);
        if (FAILED(result))
        {
            ASSERT(false, "인덱스 버퍼 생성 실패");
            return E_FAIL;
        }
   // }

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
    return S_OK;
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

