#pragma once
#include "../framework.h"
#include "../Renderer/Resources/ModelData.h"

namespace renderer
{
    struct RenderPacket;
    class Renderer;
}

namespace scene
{
    class Floor
    {
    public:
        Floor(XMFLOAT2 startPoint, uint32_t gapEachLine, uint32_t numLineX, uint32_t numLineY);
        ~Floor();

        void Draw(std::vector<renderer::RenderPacket>& commandList);
    private:
        renderer::Mesh mMesh;
    };
}
