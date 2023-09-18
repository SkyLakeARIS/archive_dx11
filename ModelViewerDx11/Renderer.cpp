#include "Renderer.h"
Renderer::Renderer(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    ASSERT(device != nullptr, "do not pass nullptr");
    ASSERT(deviceContext != nullptr, "do not pass nullptr");
}

Renderer::~Renderer()
{
    
}
