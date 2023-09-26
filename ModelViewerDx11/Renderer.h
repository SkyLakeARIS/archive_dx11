#pragma once
#include "framework.h"

class Renderer final : IUnknown
{
public:

    static Renderer* GetInstance();

    // D3D
    HRESULT CreateDeviceAndSetup(DXGI_SWAP_CHAIN_DESC& swapChainDesc, HWND hWnd, uint32 height, uint32 width, bool bDebugMode);

    HRESULT CreateVertexShader(
        const WCHAR* const path
        , D3D11_INPUT_ELEMENT_DESC* const desc
        , uint32 numDescElements
        , ID3D11VertexShader** const outVertexShader
        , ID3D11InputLayout** const outInputLayout);

    HRESULT CreatePixelShader(const WCHAR* const path, ID3D11PixelShader** const outPixelShader);

    HRESULT CreateTextureResource(
        const WCHAR* fileName
        , const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc
        , ID3D11ShaderResourceView** outShaderResourceView);

    void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) const;

    void ClearScreen();
    void Present() const;

    //
    static void    CheckLiveObjects();
    HRESULT        CheckDeviceLost();

    void    Cleanup();

    // COM
    ULONG   AddRef() override;
    ULONG   Release() override;
    HRESULT QueryInterface(const IID& riid, void** ppvObject) override;


    // getter
    ID3D11Device*           GetDevice() const;
    ID3D11DeviceContext*    GetDeviceContext() const;
private:
    Renderer();
    ~Renderer();

    HRESULT compileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

   // bool SetInputLayout();
public:

    ID3D11ShaderResourceView* DefaultTexture;
private:
    static Renderer* mInstance;

    ULONG mRefCount;
    // D3D Device
    ID3D11Device* mDevice;
    ID3D11DeviceContext* mDeviceContext;
    IDXGISwapChain* mSwapChain;
    ID3D11RenderTargetView* mBackBufferRTV;
    ID3D11DepthStencilView* mDepthStencilView;

    ID3D11Texture2D* mDepthStencilTexture;

    //
    
    // window
    HWND mhWindow;
    uint32 mWindowHeight;
    uint32 mWindowWidth;

};
