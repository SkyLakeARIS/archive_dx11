#pragma once
#include "framework.h"

using namespace DirectX;

class Camera final
{
public:
    Camera(XMVECTOR vEye, XMVECTOR vLookAt, XMVECTOR vUp);
    virtual ~Camera();

    void RotateAxis(float yawRad, float pitchRad);
    
    void AddRadiusSphere(float scaleFactor);

    inline XMMATRIX GetViewMatrix() const;
    inline XMMATRIX GetProjectionMatrix();
private:

    void makeViewMatrix();

private:
    XMFLOAT2    mAnglesRad;

    XMVECTOR    mvEye;
    XMVECTOR    mvLookAtCenter;
    XMVECTOR    mvUp;

    XMVECTOR    mvForward;     // view vector
    XMVECTOR    mvRight;       // right, cross vector

    float       mRadiusOfSphere; // sphere size, radius length

    XMMATRIX    mMatView;
    XMMATRIX    mMatProjection;

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