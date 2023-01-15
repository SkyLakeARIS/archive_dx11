#include "Camera.h"
#include "cmath"

Camera::Camera(XMVECTOR vEye, XMVECTOR vLookAt, XMVECTOR vUp)
    : mvEye(vEye)
    , mvLookAtCenter(vLookAt)
    , mvUp(vUp)
    , mRadiusOfSphere(0.0f)
{
    // ���� ĳ���� ���� ���� �ϼ��� �Ǹ�, ĳ���Ϳ� ���� ��ġ ��ǥ ���� �޾Ƽ�
    // �߽����� ����ϴ� ���� ���� ��. ������ ���� �ָ��� ( eye�� center�� lookat���� Ŀ�� ���� �ٶ󺸵��� �Ǿ�����.)
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

    // pitch�� Ŭ����, yaw�� ��ȯ
    // pitch limit�� -85.0~85.0
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

    // ���� r�� 1�� ���� ��ü�� �����ϰ� ��� ��, radius��ŭ �Ÿ��� �����Ѵ�.
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
