#pragma once
#include "Camera.h"
#include "framework.h"

// MEMO: 만들어놓고 XMVECTOR 비율이 더 높으면 XMFLOAT3->XMVECTOR로
class Light final // working like directional light
{
public:
    Light(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 color);
    ~Light();

    void Initialize();
    void Update(Camera* camera);
    void Draw(double deltaTime);

    void Move(double deltaTime, float direction);


    void SetDirection(XMFLOAT3 dir);
    void SetColor(XMFLOAT3 color);

    XMFLOAT4 GetDirection() const;
    XMFLOAT4 GetPosition() const;
    XMFLOAT4 GetColor() const;
    const XMMATRIX* const GetViewProjMatrix() const;

private:

    void updateMatrices();

private:

    XMFLOAT3 mPosition;
    XMFLOAT3 mDirection;
    XMFLOAT3 mColor;

    // for shadow-map
    XMMATRIX mMatView;
    XMMATRIX mMatProj;
    XMMATRIX mMatViewProj;

    ID3D11Buffer* mCbMatWorld;

    class Plane* mMesh;
    XMMATRIX mMatWorld;
    ID3D11BlendState* mBlendState;
};
