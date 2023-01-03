#include "Camera.h"


Camera::Camera(XMVECTOR vEye, XMVECTOR vLookAt, XMVECTOR vUp)
    : mvEye(vEye)
    , mvLookAt(vLookAt)
    , mvUp(vUp)
{
    mvForward = XMVectorSubtract(mvLookAt, mvEye);
    mvRight = XMVector3Cross(mvUp, mvForward);

    makeViewMatrix();
}

Camera::~Camera()
{
    
}

void Camera::MoveForward(float deltaTime)
{
    XMVECTOR vMoveToFront = XMVector3Normalize(mvForward);

    vMoveToFront = XMVectorScale(vMoveToFront, deltaTime);

    mvEye = XMVectorAdd(mvEye, vMoveToFront);
    mvLookAt = XMVectorAdd(mvLookAt, vMoveToFront);

    makeViewMatrix();
}

void Camera::MoveRight(float deltaTime)
{
    XMVECTOR vMoveToRight = XMVector3Normalize(mvRight);

    vMoveToRight = XMVectorScale(vMoveToRight, deltaTime);

    mvEye = XMVectorAdd(mvEye, vMoveToRight);
    mvLookAt = XMVectorAdd(mvLookAt, vMoveToRight);

    makeViewMatrix();
}

void Camera::RotateYAxis(float deltaTime)
{
    // 화면에서 움직인 마우스 거리를 호의 길이라고 한다면,
    // 반지름은 벡터의 길이(크기) , y축 회전이니까 forward벡터를 활용
    // radian = r = l(호의 길이)
    // radian*r = r*l (l은 배수로 해야함.)
    // 다르게 쓰면 theta = l/r => theta*r = l


    XMMATRIX matRotate = XMMatrixRotationAxis(mvUp, deltaTime);
    mvForward = XMVector3TransformCoord(mvForward, matRotate);
    auto oldLookat = mvLookAt;
    mvLookAt = mvForward + mvEye;

    mvRight = XMVector3Cross(mvUp, mvForward);

    //mvUp = vUp;
    wchar_t a[246];
    swprintf_s(a, L"rotate y : %f %f %f %f old : %f %f %f %f\n", mvLookAt.m128_f32[0], mvLookAt.m128_f32[1], mvLookAt.m128_f32[2], mvLookAt.m128_f32[3],
        oldLookat.m128_f32[0], oldLookat.m128_f32[1], oldLookat.m128_f32[2], oldLookat.m128_f32[3]);
    std::wstring b;
    b = L"a";
    OutputDebugString(a);

    makeViewMatrix();

    //// 내가 생각했던, 원점기준으로 회전하기 방식
    //XMMATRIX matRotate = XMMatrixRotationY(deltaTime);

    //XMVECTOR vRotate;
    //XMVECTOR vRotate1;
    //XMMatrixDecompose(&vRotate1, &vRotate, &vRotate1, matRotate);
    //mvForward = XMVector3Rotate(mvForward, vRotate);

    //mvLookAt = mvForward + mvEye;

    //mvRight = XMVector3Cross(mvUp, mvForward);

    //makeViewMatrix();
}

void Camera::RotateXAxis(float deltaTime)
{
    XMMATRIX matRotate = XMMatrixRotationAxis(mvRight, deltaTime);
    mvForward = XMVector3TransformCoord(mvForward, matRotate);

    mvLookAt = mvForward + mvEye;

    mvUp = XMVector3Cross(mvForward, mvRight);
    mvUp = XMVector3Normalize(mvUp);

    wchar_t a[246];
    swprintf_s(a, L"rotate x : %f %f %f %f\n", mvUp.m128_f32[0], mvUp.m128_f32[1], mvUp.m128_f32[2], mvUp.m128_f32[3]);
    std::wstring b;
    b = L"a";
    OutputDebugString(a);

    makeViewMatrix();
}

void Camera::RotateZAxis(float deltaTime)
{
    XMMATRIX matRotate = XMMatrixRotationAxis(mvForward, deltaTime);
    //XMVECTOR vUp = XMVectorSubtract(mvUp, mvEye);
    mvUp = XMVector3TransformCoord(mvUp, matRotate);

    //mvUp = vUp;
    wchar_t a[246];
    swprintf_s(a, L"rotate z : %f %f %f %f\n", mvUp.m128_f32[0], mvUp.m128_f32[1], mvUp.m128_f32[2], mvUp.m128_f32[3]);
    std::wstring b;
    b = L"a";
    OutputDebugString(a);
    //OutputDebugString(b.c_str());
    mvRight = XMVector3Cross(mvUp, mvForward);
    makeViewMatrix();
}

void Camera::RotateAxis(XMVECTOR axis, float angle)
{
    // rotate vectors 
    XMVECTOR vUp = XMVectorSubtract(mvUp, mvEye);


    mvForward = XMVector3Transform(mvForward, XMMatrixRotationAxis(mvForward, angle));
    vUp = XMVector3Transform(vUp, XMMatrixRotationAxis(mvForward, angle));

    // restore vectors's end points mTarget and mUp from new rotated vectors 
    mvLookAt = mvEye + mvForward;
    mvUp = mvEye + vUp;
    mvRight = XMVector3Cross(mvUp, mvForward);

    makeViewMatrix();

}

void Camera::makeViewMatrix()
{
    mMatView = XMMatrixLookAtLH(mvEye, mvLookAt, mvUp);
}
