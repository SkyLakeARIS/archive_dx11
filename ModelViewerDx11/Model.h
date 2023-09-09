#pragma once
#include "Renderer.h"
#include "framework.h"

class ModelImporter;

constexpr size_t MESH_NAME_LENGTH = 128;

struct Vertex
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT2 TexCoord;
};

struct Mesh
{
    WCHAR Name[MESH_NAME_LENGTH];
    std::vector<Vertex> Vertex;
    std::vector<unsigned int> IndexList;
    bool HasTexture;
    ID3D11ShaderResourceView* Texture;
    uint8 NumTexuture;
};

enum eShader
{
    BASIC,
    OUTLINE,
};
class Model
{

public:
    Model(Renderer* renderer, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    ~Model();

    void                Draw();

    HRESULT             SetupMesh(ModelImporter& importer);

    // 일단 여기에 때려 박는다.
    // 하다보면 다른 곳으로 빼야할 부분이 보이겠지...
    HRESULT             SetupShader(eShader shaderType, ID3D11VertexShader* tempVsShaderToSet, ID3D11PixelShader* tempPsShaderToSet, ID3D11InputLayout* tempInputLayout);

    //void                UpdateVertexBuffer(Vertex* buffer, size_t bufferSize, size_t startIndex);
    //void                UpdateIndexBuffer(unsigned int* buffer, size_t bufferSize, size_t startIndex);

    size_t              GetMeshCount() const;
    const WCHAR*        GetMeshName(size_t meshIndex) const;

    size_t              GetVertexCount(size_t meshIndex) const;
    size_t              GetIndexListCount(size_t meshIndex) const;

    XMFLOAT3            GetCenterPoint() const;
private:

    void prepare();

private:

    Renderer* mRenderer;
    ID3D11Device* mDevice;
    ID3D11DeviceContext* mDeviceContext;

    size_t mNumMesh;
    size_t mNumVertex;

    std::vector<Mesh> mMeshes;

    ID3D11Buffer* mVertexBuffers;
    ID3D11Buffer* mIndexBuffers;

    XMFLOAT3 mCenterPosition;
    XMMATRIX mWorldMatrix;

    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11InputLayout* mInputLayout;

    Vertex* vertices;
    uint32* indices;
};


/*
 *  anbi TM
 */
 ////  최상위
 //mMatRootTM = XMMATRIX(
 //    39.370080, 0.000000, 0.000000, 0.000000,
 //    0.000000, -0.000002, -39.370080, 0.000000,
 //    0.000000, 39.370080, -0.000002, 0.000000,
 //    0.000000, 0.000000, 0.000000, 1.000000);

 //// 그다음
 //mMatSubTM = XMMATRIX(
 //    1.000000, 0.000000, 0.000000, 0.000000,
 //    0.000000, 1.000000, 0.000000, 0.000000,
 //    0.000000, -0.000000, 1.000000, 0.000000,
 //    0.000000, 0.000000, 0.000000, 1.000000);

 //mMatBodyTM = XMMATRIX(
 //    1.000000, 0.000000, 0.000000, 0.000000,
 //    0.000000, -0.000000, 1.000000, 0.000000,
 //    0.000000, -1.000000, -0.000000, 0.000000,
 //    0.000000, 0.000000, 0.000000, 1.000000);

 //mMatFaceTM = XMMATRIX(
 //    1.000000, 0.000000, 0.000000, 0.000000,
 //    0.000000, -0.000000, 1.000000, 0.000000,
 //    0.000000, -1.000000, -0.000000, 0.000000,
 //    0.000000, 0.000000, 0.000000, 1.000000);

 //mMatHairTM = XMMATRIX(
 //    1.000000, 0.000000, 0.000000, 0.000000,
 //    0.000000, -0.000000, 1.000000, 0.000000,
 //    0.000000, -1.000000, -0.000000, 0.000000,
 //    0.000000, 0.000000, 0.000000, 1.000000);