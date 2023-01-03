#pragma once
#include "framework.h"

using namespace DirectX;

class Camera final
{
public:
    Camera(XMVECTOR vEye, XMVECTOR vLookAt, XMVECTOR vUp);
    virtual ~Camera();

    void MoveForward(float deltaTime);
    void MoveRight(float deltaTime);

    void RotateYAxis(float deltaTime);
    void RotateXAxis(float deltaTime);
    void RotateZAxis(float deltaTime);

    void RotateAxis(XMVECTOR axis, float angle);

    inline XMMATRIX GetViewMatrix() const;
    inline XMMATRIX GetProjectionMatrix();
private:

    void makeViewMatrix();

private:
    XMVECTOR mvEye;
    XMVECTOR mvLookAt;
    XMVECTOR mvUp;

    XMVECTOR mvForward; // view vector
    XMVECTOR mvRight;   // right, cross vector


    XMMATRIX mMatView;
    XMMATRIX mMatProjection;
};

inline XMMATRIX Camera::GetViewMatrix() const
{
    return mMatView;
}

inline XMMATRIX Camera::GetProjectionMatrix()
{
    INT32 width = 1024;
    INT32 height = 720;
    mMatProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.001f, 1000.0f);
    return mMatProjection;
}