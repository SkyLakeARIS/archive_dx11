#pragma once
#include "framework.h"
#include "Model.h"

class Plane final
{
public:

    Plane();
    ~Plane();

    void Draw(ID3D11DeviceContext* const deviceContext, ID3D11ShaderResourceView* const srv);

    void GetVertices(std::vector<Vertex>* const outVertices) const;
    void GetIndices(std::vector<unsigned int>* const outIndices) const;

private:
    Mesh mMesh;
};
