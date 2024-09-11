#include "Light.h"

Light::Light(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 color)
    :mPosition(pos)
    , mDirection(dir)
    , mColor(color)
    , mMatView(XMMatrixIdentity())
    , mMatProj(XMMatrixIdentity())
    , mMatViewProj(XMMatrixIdentity())
{
    updateMatrices();
}

void Light::SetDirection(XMFLOAT3 dir)
{
    mDirection = dir;

    updateMatrices();
}

void Light::SetColor(XMFLOAT3 color)
{
    mColor = color;
}

XMFLOAT4 Light::GetDirection() const
{
    return XMFLOAT4(mDirection.x, mDirection.y, mDirection.z, 1.0f);
}

XMFLOAT4 Light::GetPosition() const
{
    return XMFLOAT4(mPosition.x, mPosition.y, mPosition.z, 1.0f);
}

XMFLOAT4 Light::GetColor() const
{
    return XMFLOAT4(mColor.x, mColor.y, mColor.z, 1.0f);
}

const XMMATRIX* const Light::GetViewProjMatrix() const
{
    return &mMatViewProj;
}

void Light::updateMatrices()
{
    mMatView = XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mDirection), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
   //mMatProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0, 69.0f, 120.0f);
    mMatProj = XMMatrixOrthographicOffCenterLH(-1.0f, 1.0f, -1.0f, 1.0f, 69.0f, 100.0f);
    mMatViewProj = mMatView * mMatProj;

}
