#pragma once
#include "../framework.h"
#include "../Renderer/Resources/Mesh.h"

namespace renderer
{
    struct RenderPacket;
    class Renderer;
}

namespace scene
{
    class Camera;

    // MEMO: 만들어놓고 XMVECTOR 비율이 더 높으면 XMFLOAT3->XMVECTOR로
    class Light final // working like directional light
    {
        enum eCascadeLevel
        {
            Level_4 = 5 // near - 1 - 2- 3- 4 - far
        };
    public:
        Light(XMFLOAT3 pos, XMFLOAT3 dir, XMFLOAT3 color, Camera* camera, float nearPlane, float farPlane);
        ~Light();

        void DrawDebug(std::vector<renderer::RenderPacket>& commandList);

        void SetupCascade(renderer::Renderer& renderer);

        XMFLOAT4 GetDirection() const;
        XMFLOAT3 GetPosition() const;
        XMFLOAT4 GetColor() const;
        XMMATRIX GetViewProjMatrix() const;

    private:

        void updateMatrices();


        void getPointsFromMatrix(XMMATRIX* matView, float nearPlane, float farPlane, XMMATRIX* const outMatLightView, XMMATRIX* const outMatLightProj, renderer::Renderer& renderer);
    private:

        XMFLOAT3 mPosition;
        XMFLOAT3 mDirection;
        XMFLOAT3 mColor;

        // for shadow-map
        XMMATRIX mMatView;
        XMMATRIX mMatProj;
        XMMATRIX mMatViewProj;

        std::vector<XMFLOAT3> mLines;
        renderer::Mesh mMeshDebug;

        float mNearPlane;
        float mFarPlane;
        Camera* mCamera;
        XMMATRIX mMatLightViews[eCascadeLevel::Level_4];
        XMMATRIX mMatLightProjs[eCascadeLevel::Level_4];
        float mCascadePlaneDistances[eCascadeLevel::Level_4 + 1];
    };
}
