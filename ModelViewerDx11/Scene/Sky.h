#pragma once
#include "../framework.h"
#include "../Renderer/Resources/ModelData.h"

namespace renderer
{
    struct RenderPacket;
    class Renderer;
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

        HRESULT Initialize(uint32 latLines, uint32 lonLines, renderer::TextureManager* const texManager);

        void Draw(std::vector<renderer::RenderPacket>& renderer);
        void Update();

    private:
        Camera* mCamera;
        renderer::Mesh mMesh;
        XMMATRIX mWorld;
    };
}
