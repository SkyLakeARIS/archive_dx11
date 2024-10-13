#pragma once
#include "Renderer.h"
#include "Camera.h"

class ModelImporter;

constexpr size_t MESH_NAME_LENGTH = 128;

struct Vertex // 4bytes align
{
    XMFLOAT3 Position;
    XMFLOAT2 TexCoord;
    XMFLOAT3 Normal;
    float    Reserve1;
};

// 모델링 프로그램에서 미리 계산된 값으로 사용
struct Material // 16 bytes align
{
    XMFLOAT3 Diffuse;
    float    Reserve0;
    XMFLOAT3 Ambient;
    float    Reserve1;
    XMFLOAT3 Specular;
    float    Reserve2;
    XMFLOAT3 Emissive;
    float    Reserve3;
    float    Opacity;       // 알파값으로 사용
    float    Reflectivity;
    float    Shininess;     // 스페큘러 거듭제곱 값
    float    Reserve4;
};

typedef Material CbMaterial;

struct Mesh
{
    /*
     * MEMO vertex/index list는 하나로 관리하고 mesh는 각 vertex, index에 대해서 offset만 가지도록 변경 (하면 또 대규모 공사인데)
     * -> draw 로직에 큰 변화는 없음. 하지만 모델에서 임포터에서 받은 메시데이터를 다시 하나로 뭉치는 작업을 임포터로 옮길 수 있음.
     * -> 근데 그 뿐이기 때문에 좀 더 고민
     */
    WCHAR Name[MESH_NAME_LENGTH];
    std::vector<Vertex> Vertex;
    std::vector<unsigned int> IndexList;
    Material Material;
    bool bLightMap;
    /*
     *  MEMO 현재 구조는 같은 파일을 여러번 로드해서 가지고 있기 때문에
     * 나중에 TextureManager가 가지고 있어야 할 듯. ( ID-SRV, ID - 이름을 해시로 or  번호)
     * 셰이더랑 버텍스 구조 변경해야 할 수도.
     */
    ID3D11ShaderResourceView* Texture;
    ID3D11ShaderResourceView* TextureNormal;
    uint8 NumTexuture;
};

// 1. 셰이더 정리 (각 형식대로 파일 네이밍 변경 및 inputlayout등 코드 수정) - 완료
// 2. Shader 매니저, Light 클래스, 텍스쳐 매니저 제작 - 완료(임시로 Renderer에)
// TODO 3. 모델 로드할 수 있는 기능 추가
// TODO 4. 모델 로드 및 텍스쳐 등 리소스 생성 실패시에도 돌 수 있도록 Default 리소스 준비 및 적용(기본 화면은 스카이박스 + 바닥면만 생성, 둘 중 안되면 단색상으로 초기화)

class Model
{
public:
    Model(Renderer* renderer, Camera* camera);
    ~Model();

    void                Draw();
    void                DrawShadow();

    void Update();

    // TODO: LightManager 만들면 제거.
    void SetLight(class Light* light);

    HRESULT             SetupMesh(ModelImporter& importer);

    void                SetHighlight(bool bSelection);
 
    size_t              GetMeshCount() const;
    const WCHAR*        GetMeshName(size_t meshIndex) const;

    size_t              GetVertexCount(size_t meshIndex) const;
    size_t              GetIndexListCount(size_t meshIndex) const;

    XMFLOAT3            GetCenterPoint() const;
private:


    void prepare();

private:

    Renderer*               mRenderer;
    ID3D11Device*           mDevice;
    ID3D11DeviceContext*    mDeviceContext;
    Camera* mCamera; // 나중에 모델에 카메라를 붙이도록(상호참조해야 할 것 같음. 아니면 다른 방법)

    size_t mNumMesh;
    size_t mNumVertex;

    std::vector<Mesh> mMeshes;

    Vertex* mVertices;
    uint32* mIndices;

    ID3D11Buffer* mVertexBuffers;
    ID3D11Buffer* mIndexBuffers;

    ID3D11Buffer* mCbMatWorld;

    XMFLOAT3 mCenterPosition;
    XMMATRIX mMatWorld;
    XMMATRIX mMatRotation;
    XMMATRIX mMatScale;

    Light* mLight;

    // texture
    ID3D11SamplerState* mSamplerState;


    bool mbHighlight;
    bool mbActiveEmissive;
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