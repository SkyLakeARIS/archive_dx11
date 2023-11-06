#pragma once
#include "Model.h"

class Sky
{
private:
    struct CbWVP
    {
        XMMATRIX WVP;
    };
public:

    Sky(Renderer& renderer, Camera& camera);
    ~Sky();

    HRESULT Initialize(uint32 latLines, uint32 lonLines);
    void Draw();

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
    ID3D11InputLayout* mInputLayout;

    ID3D11Buffer* mCbMatWVP;

    ID3D11VertexShader* mVs;
    ID3D11PixelShader* mPs;

    Mesh mMesh; // sphere model
    ID3D11SamplerState* mSampler;

    uint32 mLatLines;
    uint32 mLonLines;


};

