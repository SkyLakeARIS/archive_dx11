#pragma once
#include "Renderer.h"
#include "Camera.h"

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
    /*
     * vertex/index list는 하나로 관리하고 mesh는 각 vertex, index에 대해서 offset만 가지도록 변경 (하면 또 대규모 공사인데)
     */
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
    struct CbBasic
    {
        XMMATRIX World;
        XMMATRIX WVP;
    };
    struct CbOutline
    {
        XMMATRIX WVP;
    };
    struct alignas(16) CbOutlineWidth // CB생성시 byteWidth는 16바이트 배수 셰이더 고정값으로 할지, 그냥 12바이트 낭비할지
    {
        float OutlineWidth;
    };
public:
    Model(Renderer* renderer, Camera* camera);
    ~Model();

    void                Draw();

    HRESULT             SetupMesh(ModelImporter& importer);

    // 일단 여기에 때려 박는다.
    // 하다보면 다른 곳으로 빼야할 부분이 보이겠지...
    HRESULT             SetupShader(eShader shaderType, ID3D11VertexShader* tempVsShaderToSet, ID3D11PixelShader* tempPsShaderToSet, ID3D11InputLayout* tempInputLayout);
    HRESULT             SetupShaderFromRenderer(); // 셰이더 관리하는 무언가가 나오기 전까지


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
    Camera* mCamera; // 나중에 모델에 카메라를 붙이도록(상호참조해야 할 것 같음. 아니면 다른 방법)

    size_t mNumMesh;
    size_t mNumVertex;

    std::vector<Mesh> mMeshes;

    Vertex* vertices;
    uint32* indices;

    ID3D11Buffer* mVertexBuffers;
    ID3D11Buffer* mIndexBuffers;

    // CB를 어떻게 만드는게 좋을지 고민 or 더 경험이 필요
    ID3D11Buffer* mCbBasicShader;

    ID3D11Buffer* mCbOutlineShader;
    ID3D11Buffer* mCbOutlineWidth;

    XMFLOAT3 mCenterPosition;
    XMMATRIX mMatWorld;
    XMMATRIX mMatRotation;
    XMMATRIX mMatScale;


    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11InputLayout* mInputLayout;

    // texture
    ID3D11SamplerState* mSamplerState;


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