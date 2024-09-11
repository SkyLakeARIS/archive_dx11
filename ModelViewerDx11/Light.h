#pragma once
#include "framework.h"

// TODO: 만들어놓고 XMVECTOR 비율이 더 높으면 XMFLOAT3->XMVECTOR로
class Light final // working like directional light
{
public:
    Light(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 color);
    ~Light() = default;
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

    XMMATRIX mMatView;
    XMMATRIX mMatProj;
    XMMATRIX mMatViewProj;
};
