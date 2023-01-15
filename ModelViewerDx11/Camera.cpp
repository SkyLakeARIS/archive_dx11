#include "Camera.h"
#include "cmath"

Camera::Camera(XMVECTOR vEye, XMVECTOR vLookAt, XMVECTOR vUp)
    : mvEye(vEye)
    , mvLookAtCenter(vLookAt)
    , mvUp(vUp)
    , mRadiusOfSphere(0.0f)
{
    // 실제 캐릭터 모델을 만들어서 완성이 되면, 캐릭터에 대한 위치 좌표 등을 받아서
    // 중심점을 계산하는 것이 좋을 듯. 지금은 조금 애매함 ( eye가 center인 lookat보다 커야 앞을 바라보도록 되어있음.)
    mvForward = XMVectorSubtract(mvEye, mvLookAtCenter);
    mvForward = XMVector3Normalize(mvForward);

    XMFLOAT3 distance;
    XMStoreFloat3(&distance, mvEye);
    mRadiusOfSphere = sqrtf(distance.x * distance.x + distance.y * distance.y + distance.z * distance.z);
    
    mvRight = XMVector3Cross(mvUp, mvForward);
    mvRight = XMVector3Normalize(mvRight);

    mAnglesRad.x = 0.0f;
    mAnglesRad.y = -(distance.x/distance.z);

    makeViewMatrix();
}

Camera::~Camera()
{
    
}


void Camera::RotateAxis(float yawRad, float pitchRad)
{
    mAnglesRad.x += yawRad;         // pi, yaw
    mAnglesRad.y += pitchRad;       // theta, pitch

    // pitch만 클램프, yaw는 순환
    // pitch limit는 -85.0~85.0
    constexpr float PITCH_LIMIT = 0.261799; // 15 degree
    if(mAnglesRad.x >= XM_2PI)
    {
        mAnglesRad.x -= XM_2PI;
    }
    else if (mAnglesRad.x < 0.0f)
    {
        mAnglesRad.x += XM_2PI;
    }

    if (mAnglesRad.y > (XM_PI - PITCH_LIMIT))
    {
        mAnglesRad.y = XM_PI - PITCH_LIMIT;
    }
    else if (mAnglesRad.y < PITCH_LIMIT)
    {
        mAnglesRad.y = PITCH_LIMIT;
    }

    // 단위 r이 1인 단위 구체로 생각하고 계산 후, radius만큼 거리를 조정한다.
    XMFLOAT3 position;
    position.x = sin(mAnglesRad.y) * sin(mAnglesRad.x);
    position.y = cos(mAnglesRad.y);
    position.z = -(sin(mAnglesRad.y) * cos(mAnglesRad.x));

    XMMATRIX matNewPosition = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
    
    mvEye = XMVector3TransformCoord(mvEye, matNewPosition);

    mvEye = XMVector3Normalize(mvEye)*mRadiusOfSphere;

    //mvUp = XMVector3Cross(mvForward, mvRight);


    makeViewMatrix();
}

void Camera::AddRadiusSphere(float scaleFactor)
{
    constexpr float MAX_RADIUS = 60.0f;
    constexpr float MIN_RADIUS = 5.0f;

    mRadiusOfSphere += scaleFactor;

    if(mRadiusOfSphere > MAX_RADIUS)
    {
        mRadiusOfSphere = MAX_RADIUS;
    }
    else if(mRadiusOfSphere < MIN_RADIUS)
    {
        mRadiusOfSphere = MIN_RADIUS;
    }

    mvEye = XMVector3Normalize(mvEye) * mRadiusOfSphere;
    makeViewMatrix();
}

void Camera::makeViewMatrix()
{
    mMatView = XMMatrixLookAtLH(mvEye, mvLookAtCenter, mvUp);
}
