#pragma once
#include "../Resources/Mesh.h"

namespace scene
{
    class Camera;
}

namespace renderer
{
    struct RenderPacket;
    class BufferManager;


    class Model
    {
    public:
        Model(scene::Camera* camera, BufferManager* bufferManager);
        ~Model();

        void Draw(std::vector<renderer::RenderPacket>& commandList);
        void DrawShadow(std::vector<renderer::RenderPacket>& commandList);

        void Update();

        void SetMesh(const Mesh& mesh);
        void SetCenterPoint(XMFLOAT4& centerPoint);

        void SetHighlight(bool bSelection);

        XMFLOAT3 GetCenterPoint() const;

    private:
        BufferManager* mBufferManager;

        Mesh mMesh;

        XMFLOAT3 mCenterPosition;
        XMMATRIX mMatWorld;
        XMMATRIX mMatRotation;
        XMMATRIX mMatScale;

        bool mbHighlight;
        bool mbActiveEmissive;
    };
}
