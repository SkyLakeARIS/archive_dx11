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
        CullBack,
        NumRaster,
    };

    enum class eInputLayout : uint8_t
    {
        Basic,  // pos, normal, tex
        PT,  // pos, tex
        Simple, // pos
        NumInputlayout
    };

    enum class eShader : uint32_t
    {
        Basic,
        Outline,
        LightMap,
        Skybox,
        Shadow,
        BasicWithShadow,
        RenderToTexture,
        NumShader
    };

    // RenderTarget, DepthStencil 
    enum class eRenderTarget : uint8_t
    {
        Default,
        Shadow,
        NumRenderTarget
    };

private:
    enum class eVertexShader : uint32_t
    {
        VsBasic,
        VsBasicWithShadow,
        VsOutline,
        VsRenderToTexture,
        VsSimple,
        VsSkybox,
        NumVertexShader
    };

    enum class ePixelShader : uint32_t
    {
        PsBasic,
        PsBasicWithShadow,
        PsOutline,
        PsShadow,
        PsSkybox,
        PsRenderToTexture,
        PsLightMap,
        NumPixelShader
    };

    struct ShaderMap
    {
        eShader Type;
        eVertexShader VsIndex;
        ePixelShader PsIndex;
    };


    struct RenderTargetDepthStencilMap
    {
        uint32_t RenderTargetIndex;
        uint32_t NumViews;
        uint32_t DepthStencilIndex;
    };
    typedef RenderTargetDepthStencilMap RtvDsMap;

public:

    static Renderer* GetInstance();

    // D3D
    HRESULT CreateDeviceAndSetup(DXGI_SWAP_CHAIN_DESC& swapChainDesc, HWND hWnd, uint32 height, uint32 width, bool bDebugMode);

    HRESULT CreateRenderTargetView(ID3D11Texture2D* const texture, D3D11_RENDER_TARGET_VIEW_DESC* const desc, ID3D11RenderTargetView** outRtv, const char* const debugTag = "NO_INFO");

    HRESULT CreateDepthStencilView(ID3D11Texture2D* const texture, D3D11_DEPTH_STENCIL_VIEW_DESC* const desc, ID3D11DepthStencilView** outDs, const char* const debugTag = "NO_INFO");

    // 그림자 매핑을 위한 설계
    void SetViewport(bool bFullScreen);

    // TODO : 정리필요. 
    HRESULT CreateShadowRenderTarget();


    // init - program

    HRESULT PrepareRender();


    // Cate : shader

    HRESULT CreateVertexShaderAndInputLayout(
        const WCHAR* const path
        , D3D11_INPUT_ELEMENT_DESC* const desc
        , uint32 numDescElements
        , ID3D11VertexShader** const outVertexShader
        , ID3D11InputLayout** const outInputLayout);

    HRESULT CreateInputLayout(
        const WCHAR* const path
        , D3D11_INPUT_ELEMENT_DESC* const desc
        , uint32 numDescElements
        , eInputLayout type
        , ID3D11InputLayout** const outInputLayout);

    HRESULT CreateVertexShader(
        const WCHAR* const path
        , ID3D11VertexShader** const outVertexShader);

    HRESULT CreatePixelShader(const WCHAR* const path, ID3D11PixelShader** const outPixelShader);


    // Cate : texture 
    HRESULT CreateTexture2D(D3D11_TEXTURE2D_DESC& desc, ID3D11Texture2D** outTex, const char* tag);

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

    // Renderer 
    void ClearScreenAndDepth(eRenderTarget type);
    void ClearDepthBuffer();
    void Present() const;

    void    Cleanup();

    HRESULT        CheckDeviceLost();

    // Debug
    static void    CheckLiveObjects();

    // COM
    ULONG   AddRef() override;
    ULONG   Release() override;
    HRESULT QueryInterface(const IID& riid, void** ppvObject) override;


    //  D3D state
    void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) const;

    void SetRenderTargetTo(eRenderTarget type);
    void SetInputLayoutTo(eInputLayout type) const;
    void SetShaderTo(eShader type);
    void SetRasterState(eRasterType type);

    void SetDepthStencilState(bool bSkybox); // 현재는 스카이박스만 사용하므로

    // getter
    ID3D11Device*           GetDevice() const;
    ID3D11DeviceContext*    GetDeviceContext() const;

    void                    GetWindowSize(uint32& outWidth, uint32& outHeight) const;
    HWND                    GetWindowHandle() const;

    ID3D11ShaderResourceView*        GetShadowTexture();
private:
    Renderer();
    ~Renderer();

    HRESULT compileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

    HRESULT createRasterState();

    HRESULT setupShaders();

public:

    ID3D11ShaderResourceView*   DefaultTexture;
private:

    static Renderer*            mInstance;

    ULONG                       mRefCount;

    // D3D Device
    ID3D11Device*               mDevice;
    ID3D11DeviceContext*        mDeviceContext;

    // shader
    ShaderMap           mShaderMapTable[static_cast<uint32_t>(eShader::NumShader)]; // combine vs-ps pairs
    ID3D11VertexShader* mVertexShadersList[static_cast<uint32_t>(eVertexShader::NumVertexShader)];
    ID3D11PixelShader*  mPixelShaderList[static_cast<uint32_t>(ePixelShader::NumPixelShader)];
    ID3D11InputLayout*  mInputLayoutList[static_cast<uint32_t>(eInputLayout::NumInputlayout)];

    // 
    IDXGISwapChain*             mSwapChain;

    ID3D11Texture2D*            mDepthStencilTexture;
    ID3D11DepthStencilState*    mSkyboxDepthStencil;

    // render target, depthStencil
    // 일단은 쉽게 무조건 1:1매핑으로 (nullptr 처리는 나중에 최적화)
    ID3D11RenderTargetView* mRenderTargetViewList[static_cast<uint8_t>(eRenderTarget::NumRenderTarget)];
    ID3D11DepthStencilView* mDepthStencilViewList[static_cast<uint8_t>(eRenderTarget::NumRenderTarget)];
    RtvDsMap mRtvDsMapTable[static_cast<uint8_t>(eRenderTarget::NumRenderTarget)]; // combine rtv - depth-stencil pairs

    // shadow
    ID3D11Texture2D*            mTexShadow;
    ID3D11Texture2D*            mTexColor;
    ID3D11ShaderResourceView*     mShadowSrv;
    D3D11_VIEWPORT mViewportFull;
    D3D11_VIEWPORT mViewportTex;

    // raster state
    ID3D11RasterizerState*      mRasterStates[static_cast<uint32>(eRasterType::NumRaster)]; // 0: back cull, 1: front cull

    //
    
    // window
    HWND        mhWindow;
    uint32      mWindowHeight;
    uint32      mWindowWidth;

};
