#pragma once
#include "../Renderer/Resources/Mesh.h"

namespace renderer
{
    struct RenderPacket;
    class TextureManager;
}

namespace scene
{

    class Sky
    {
    public:

        Sky();
        ~Sky();

        HRESULT Initialize(uint32_t latLines, uint32_t lonLines, renderer::TextureManager* const texManager);

        void SubmitCommand(std::vector<renderer::RenderPacket>& commandList);
        void Update(const XMFLOAT3& cameraPosition);

    private:
        renderer::Mesh mMesh;
        XMMATRIX mWorld;
    };
}
