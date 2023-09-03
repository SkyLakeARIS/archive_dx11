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
    mDevice->Release();
    mDevice = nullptr;
    mDeviceContext->Release();
    mDeviceContext = nullptr;
}

void Model::Draw()
{
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

void Model::SetupMesh(ModelImporter& importer)
{
    const std::vector<Mesh>& meshes = *importer.GetMesh();

    mNumMesh = meshes.size();
    mMeshes.reserve(mNumMesh);
    for(size_t meshIndex = 0; meshIndex < mNumMesh; ++meshIndex)
    {
        mMeshes.push_back(Mesh());
        Mesh& mesh = mMeshes.back();

        mNumVertex = meshes[meshIndex].Vertex.size();
        mMeshes[meshIndex].Vertex.reserve(mNumVertex);
        for (size_t vertexIndex = 0; vertexIndex < mNumVertex; ++vertexIndex)
        {
            mesh.Vertex.push_back(Vertex());
            Vertex& vertex = mesh.Vertex.back();

            vertex.Position = meshes[meshIndex].Vertex[vertexIndex].Position;
            vertex.Normal = meshes[meshIndex].Vertex[vertexIndex].Normal;
            vertex.TexCoord = meshes[meshIndex].Vertex[vertexIndex].TexCoord;
        }

        const int numIndexList = meshes[meshIndex].IndexList.size();
        mMeshes[meshIndex].IndexList.reserve(numIndexList);
        for (size_t index = 0; index < numIndexList; ++index)
        {
            mesh.IndexList.push_back(meshes[meshIndex].IndexList[index]);
        }

        wcscpy_s(mesh.Name, MESH_NAME_LENGTH, meshes[meshIndex].Name);
        mesh.Name[MESH_NAME_LENGTH - 1] = (WCHAR)L"\n";

        mesh.Texture = meshes[meshIndex].Texture;
        mesh.NumTexuture = meshes[meshIndex].NumTexuture;
        mesh.HasTexture = meshes[meshIndex].HasTexture;

    }

}

void Model::UpdateVertexBuffer(Vertex* buffer, size_t bufferSize, size_t startIndex)
{
    ASSERT(buffer != nullptr, "버퍼는 nullptr가 아니어야 합니다.");

    const size_t numMesh = mMeshes.size();
    size_t offset = 0;
    for (size_t meshIndex = 0; meshIndex < numMesh; ++meshIndex)
    {
        const size_t numVertex = mMeshes[meshIndex].Vertex.size();
        for (size_t vertexIndex = 0; vertexIndex < numVertex; ++vertexIndex)
        {
            const size_t bufferIndex = startIndex + offset + vertexIndex;
            buffer[bufferIndex].Position = mMeshes[meshIndex].Vertex[vertexIndex].Position;
            buffer[bufferIndex].Normal = mMeshes[meshIndex].Vertex[vertexIndex].Normal;
            buffer[bufferIndex].TexCoord = mMeshes[meshIndex].Vertex[vertexIndex].TexCoord;
        }
        offset += numVertex;
    }
    
}

void Model::UpdateIndexBuffer(unsigned int* buffer, size_t bufferSize, size_t startIndex)
{
    ASSERT(buffer != nullptr, "버퍼는 nullptr가 아니어야 합니다.");

    const size_t numMesh = mMeshes.size();
    size_t offset = 0;
    for (size_t meshIndex = 0; meshIndex < numMesh; ++meshIndex)
    {
        const size_t numIndex = mMeshes[meshIndex].IndexList.size();
        for (size_t index = 0; index < numIndex; ++index)
        {
            const size_t bufferIndex = startIndex + offset + index;
            buffer[bufferIndex] = mMeshes[meshIndex].IndexList[index];
        }
        offset += numIndex;
    }

}

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

