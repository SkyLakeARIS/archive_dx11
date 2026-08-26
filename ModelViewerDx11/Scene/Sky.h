#pragma once
#include "../framework.h"
#include "../Renderer/Resources/Mesh.h"

namespace renderer
{
    struct RenderPacket;
    class TextureManager;
}

namespace scene
{
    class Camera;

    class Sky
    {
    public:

        Sky(Camera& camera);
        ~Sky();

        HRESULT Initialize(uint32_t latLines, uint32_t lonLines, renderer::TextureManager* const texManager);

        void Draw(std::vector<renderer::RenderPacket>& commandList);
        void Update();

    private:
        Camera* mCamera;
        renderer::Mesh mMesh;
        XMMATRIX mWorld;
    };
}
