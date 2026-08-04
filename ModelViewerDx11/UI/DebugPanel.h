#pragma once
#include "../Renderer/Resources/ModelData.h"

namespace renderer
{
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

        void SetDebugType(HashID texHash, int16_t serialID);
    private:
        renderer::Mesh mMesh;
    };
}
