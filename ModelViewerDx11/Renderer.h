#pragma once
#include "framework.h"

class Renderer final : IUnknown
{
public:
    enum class eRasterType
    {
        Basic,
        Outline,
        Skybox,
        RasterCount,
    };

public:

    static Renderer* GetInstance();

    // D3D
    HRESULT CreateDeviceAndSetup(DXGI_SWAP_CHAIN_DESC& swapChainDesc, HWND hWnd, uint32 height, uint32 width, bool bDebugMode);

    HRESULT CreateVertexShaderAndInputLayout(
        const WCHAR* const path
        , D3D11_INPUT_ELEMENT_DESC* const desc
        , uint32 numDescElements
        , ID3D11VertexShader** const outVertexShader
        , ID3D11InputLayout** const outInputLayout);

    HRESULT CreateVertexShader(
        const WCHAR* const path
        , ID3D11VertexShader** const outVertexShader);

    HRESULT CreatePixelShader(const WCHAR* const path, ID3D11PixelShader** const outPixelShader);

    HRESULT CreateTextureResource(
        const WCHAR* fileName
        , WIC_FLAGS flag
        , const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc
        , ID3D11ShaderResourceView** outShaderResourceView);

    HRESULT CreateDdsTextureResource(
        const WCHAR* fileName
        , DDS_FLAGS flag
        , D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc
        , ID3D11ShaderResourceView** outShaderResourceView);


    void ClearScreenAndDepth();
    void ClearDepthBuffer();
    void Present() const;

    //
    static void    CheckLiveObjects();
    HRESULT        CheckDeviceLost();

    void    Cleanup();

    // COM
    ULONG   AddRef() override;
    ULONG   Release() override;
    HRESULT QueryInterface(const IID& riid, void** ppvObject) override;


    //  setter state
    void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) const;

    void SetRasterState(eRasterType type);

    void SetDepthStencilState(bool bSkybox); // 현재는 스카이박스만 사용하므로

    // getter
    ID3D11Device*           GetDevice() const;
    ID3D11DeviceContext*    GetDeviceContext() const;

    void                    GetWindowSize(uint32& outWidth, uint32& outHeight) const;
    HWND                    GetWindowHandle() const;

private:
    Renderer();
    ~Renderer();

    HRESULT compileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

    HRESULT createRasterState();

public:

    ID3D11ShaderResourceView*   DefaultTexture;
private:
    static Renderer*            mInstance;

    ULONG                       mRefCount;

    // D3D Device
    ID3D11Device*               mDevice;
    ID3D11DeviceContext*        mDeviceContext;
    IDXGISwapChain*             mSwapChain;
    ID3D11RenderTargetView*     mBackBufferRTV;
    ID3D11DepthStencilView*     mDepthStencilView;

    ID3D11Texture2D*            mDepthStencilTexture;
    ID3D11DepthStencilState*    mSkyboxDepthStencil;
    // raster state
    ID3D11RasterizerState*      mRasterStates[static_cast<uint32>(eRasterType::RasterCount)]; // 0: back cull, 1: front cull

    //
    
    // window
    HWND        mhWindow;
    uint32      mWindowHeight;
    uint32      mWindowWidth;

};
