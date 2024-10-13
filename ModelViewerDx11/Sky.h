#pragma once
#include "Model.h"

class Sky
{
public:

    Sky(Renderer& renderer, Camera& camera);
    ~Sky();

    HRESULT Initialize(uint32 latLines, uint32 lonLines);

    void Draw();
    void Update();
private:

    HRESULT createSphere(uint32 latLines, uint32 lonLines);

private:

    Renderer* mRenderer;
    Camera* mCamera;
    ID3D11Device* mDevice;
    ID3D11DeviceContext* mDeviceContext;

    XMMATRIX mWorld;

    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;

    ID3D11Buffer* mCbMatWorld;

    Mesh mMesh; // sphere model
    ID3D11SamplerState* mSampler;

    uint32 mLatLines;
    uint32 mLonLines;

};

