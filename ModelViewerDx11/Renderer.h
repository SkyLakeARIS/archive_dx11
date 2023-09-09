#pragma once
#include "framework.h"

class Renderer
{
public:
    Renderer(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    ~Renderer();

   // bool SetInputLayout();
public:

    ID3D11ShaderResourceView* DefaultTexture;
private:
    
};
