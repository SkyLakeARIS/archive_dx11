#pragma once
#include "../Renderer/Resources/ModelData.h"

namespace renderer
{
    enum class eRenderTarget : uint8_t;
    struct RenderPacket;
}

namespace ui
{
    class DebugPanel
    {
    public:
        DebugPanel(int16_t originX, int16_t originY, int16_t width, int16_t height);
        ~DebugPanel();

        void Draw(std::vector<renderer::RenderPacket>& commandList);

        void SetDebugType(renderer::eRenderTarget type);
    private:
        renderer::Mesh mMesh;
        renderer::eRenderTarget mType;
    };
}
