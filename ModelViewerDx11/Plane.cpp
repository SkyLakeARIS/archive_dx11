#include "Plane.h"

using namespace DirectX;

Plane::Plane()
{
    
    mMesh.bLightMap = false;
    Vertex vertex;

    vertex.Position = XMFLOAT3(-0.5f, -0.5f, 0.5f);
    vertex.TexCoord = XMFLOAT2(0.0f, 1.0f);
    mMesh.Vertex.push_back(vertex);

    vertex.Position = XMFLOAT3(-0.5f, 0.5f, 0.5f);
    vertex.TexCoord = XMFLOAT2(0.0f, 0.0f);
    mMesh.Vertex.push_back(vertex);

    vertex.Position = XMFLOAT3(0.5f, 0.5f, 0.5f);
    vertex.TexCoord = XMFLOAT2(1.0f, 0.0f);
    mMesh.Vertex.push_back(vertex);

    vertex.Position = XMFLOAT3(0.5f, -0.5f, 0.5f);
    vertex.TexCoord = XMFLOAT2(1.0f, 1.0f);
    mMesh.Vertex.push_back(vertex);


    const DWORD indices[] = 
    {
        0, 1, 2,
        0, 2, 3,
    };

    for(const DWORD& index : indices)
    {
        mMesh.IndexList.push_back(index);
    }
}

Plane::~Plane()
{
    
}

void Plane::GetVertices(std::vector<Vertex>* const outVertices) const
{
    outVertices->clear();

    for(auto& vertex : mMesh.Vertex)
    {
        outVertices->push_back(vertex);
    }
    
}

void Plane::GetIndices(std::vector<unsigned int>* const outIndices) const
{
    outIndices->clear();

    for (auto& index : mMesh.IndexList)
    {
        outIndices->push_back(index);
    }
}

void Plane::Draw(ID3D11DeviceContext* const deviceContext, ID3D11ShaderResourceView* const srv)
{
    deviceContext->PSSetShaderResources(0, 1, &srv);

    deviceContext->DrawIndexed(mMesh.IndexList.size(), 0, 0);
}

