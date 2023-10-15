#include "Renderer.h"

Renderer* Renderer::mInstance = nullptr;

void Renderer::CheckLiveObjects()
{
    HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
    ASSERT(dxgidebugdll != NULL, "dxgidebug.dll 로드 실패");

    decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));

    IDXGIDebug* debug;

    GetDebugInterface(IID_PPV_ARGS(&debug));

    OutputDebugStringW(L"========================== Direct3D Object ref count 메모리 누수 체크 ===============================\r\n");
    OutputDebugStringW(L"========================== 반환되지 않은 IUnknown 객체가 있을경우 아래에 나타납니다. ============================\r\n");

    debug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugStringW(L"==============================================================================================\r\n");


    debug->Release();
}

ID3D11Device* Renderer::GetDevice() const
{
    mDevice->AddRef();
    return mDevice;
}

ID3D11DeviceContext* Renderer::GetDeviceContext() const
{
    mDeviceContext->AddRef();
    return mDeviceContext;
}

void Renderer::GetWindowSize(uint32& outWidth, uint32& outHeight) const
{
    outWidth = mWindowWidth;
    outHeight = mWindowHeight;
}

HWND Renderer::GetWindowHandle() const
{
    return mhWindow;
}

Renderer::Renderer()
    : mDevice(nullptr)
    , mDeviceContext(nullptr)
    , mSwapChain(nullptr)
    , mBackBufferRTV(nullptr)
    , mhWindow(0)
    , mWindowHeight(0)
    , mWindowWidth(0)
    , DefaultTexture(nullptr)
    , mDepthStencilView(nullptr)
    , mDepthStencilTexture(nullptr)
    , mRefCount(1)
{
}

Renderer::~Renderer()
{

    SAFETY_RELEASE(mRasterStates[static_cast<uint32>(eRasterType::Basic)]);
    SAFETY_RELEASE(mRasterStates[static_cast<uint32>(eRasterType::Outline)]);

    SAFETY_RELEASE(DefaultTexture);
    SAFETY_RELEASE(mBackBufferRTV);
    SAFETY_RELEASE(mDepthStencilTexture);
    SAFETY_RELEASE(mDepthStencilView);
    SAFETY_RELEASE(mSwapChain);
    SAFETY_RELEASE(mDeviceContext);
    SAFETY_RELEASE(mDevice);

}

HRESULT Renderer::compileShaderFromFile(
    const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT result = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    result = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(result))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return result;
    }

    if (pErrorBlob)
    {
        pErrorBlob->Release();
    }

    return S_OK;
}

HRESULT Renderer::createRasterState()
{
    HRESULT result = S_OK;
    // 기본 래스터 스테이트
    D3D11_RASTERIZER_DESC basicRasterDesc;
    ZeroMemory(&basicRasterDesc, sizeof(D3D11_RASTERIZER_DESC));

    basicRasterDesc.CullMode = D3D11_CULL_BACK;
    basicRasterDesc.FillMode = D3D11_FILL_SOLID;
    basicRasterDesc.FrontCounterClockwise = false;
    result = mDevice->CreateRasterizerState(&basicRasterDesc, &mRasterStates[static_cast<uint32>(eRasterType::Basic)]);
    if (FAILED(result))
    {
        ASSERT(false, "Failed to create RasterState for basic");
    }
    // 아웃라인용 래스터 스테이트
    D3D11_RASTERIZER_DESC outlineRasterDesc;
    ZeroMemory(&outlineRasterDesc, sizeof(D3D11_RASTERIZER_DESC));

    outlineRasterDesc.CullMode = D3D11_CULL_FRONT;
    outlineRasterDesc.FillMode = D3D11_FILL_SOLID;
    outlineRasterDesc.FrontCounterClockwise = false;
    result = mDevice->CreateRasterizerState(&outlineRasterDesc, &mRasterStates[static_cast<uint32>(eRasterType::Outline)]);
    if(FAILED(result))
    {
        ASSERT(false, "Failed to create RasterState for outline");
    }

    return result;
}


Renderer* Renderer::GetInstance()
{
    if(!mInstance)
    {
        mInstance = new Renderer();
    }
    return mInstance;
}

HRESULT Renderer::CreateDeviceAndSetup(
    DXGI_SWAP_CHAIN_DESC& swapChainDesc
    , HWND hWnd
    , uint32 height
    , uint32 width
    , bool bDebugMode)
{

    mhWindow = hWnd;
    mWindowHeight = height;
    mWindowWidth = width;

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE
    };
    UINT32 numDriverTypes = ARRAYSIZE(driverTypes);

    // gpu가 지원하는 최신버전으로 맞춰질 것이다.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT32 numFeatureLevels = ARRAYSIZE(featureLevels);

    UINT32 createDeviceFlag = 0;
    if (bDebugMode)
    {
        createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
    }


    // 왠만하면 D3D_DRIVER_TYPE_HARDWARE로 정해질 것임.
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT result;
    for (UINT32 driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
    {
        result = D3D11CreateDeviceAndSwapChain(nullptr, driverTypes[driverTypeIndex], nullptr, createDeviceFlag, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc, &mSwapChain, &mDevice, &featureLevel, &mDeviceContext);
        if(SUCCEEDED(result))
        {
            break;
        }
    }

    if (FAILED(result))
    {
        ASSERT(false, "failed to create device and swapchain");
    }

    SET_PRIVATE_DATA(mDevice, "Renderer::mDevice");
    SET_PRIVATE_DATA(mDeviceContext, "Renderer::mDeviceContext");
    SET_PRIVATE_DATA(mSwapChain, "Renderer::mSwapChain");


    // setup back buffer

    // 백버퍼를 얻어와서 렌더타겟으로 설정하는 부분
    ID3D11Texture2D* backBuffer = nullptr;
    // 데스크탑 버전에서는 기본적으로 스왑체인은 하나의 백버퍼를 가지고 있고, uwp는 만들어줘야한다.
    result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(result))
    {
        return E_FAIL;
    }

    // view가 붙은것과 아닌것의 차이가 있다.
    // 어떤 리소스가 있고 그것을 사용하기 위한 파생 인터페이스. 보통 읽기전용으로 동작한다.
    // 9과달리 11에서는 어떤 원천 리소스를 가지고 어떤 방식으로 사용하냐에 따라 인터페이스가 다 나뉘어 있다고 한다.
    result = mDevice->CreateRenderTargetView(backBuffer, nullptr, &mBackBufferRTV);
    // 백버퍼의 역할은 렌더타겟으로 지정해주는 것이 끝이다.
    // refCount이기 때문에 바로 사라지는건 아니고, gRenderTargetView가 백버퍼를 가지기 때문에 이후에 사라질 것.
    backBuffer->Release();

    SET_PRIVATE_DATA(mBackBufferRTV, "Renderer::mBackBufferRTV");

    if (FAILED(result))
    {
        return E_FAIL;
    }

    // output merger, 그래픽 버퍼에 써넣는 일을 할 때 OM이 붙음.
    /*
     * 이부분 렌더타겟에 바인딩 되지 않는데 왜 호출했는지 다시 조사 필요함
     */
    mDeviceContext->OMSetRenderTargets(1, &mBackBufferRTV, nullptr);

    D3D11_VIEWPORT viewport;
    viewport.Width = (FLOAT)mWindowWidth;
    viewport.Height = (FLOAT)mWindowHeight;
    // 보통 0~1 값으로 지정한다.
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    // 렌더링될 영역을 지정. s가 붙으니까 여러개 지정가능.(모델링 프로그램을 생각)
    // rasterizer stage
    mDeviceContext->RSSetViewports(1, &viewport);

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = mWindowWidth;
    depthDesc.Height = mWindowHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;                      // 멀티샘플링 수
    depthDesc.SampleDesc.Quality = 0;                    // 멀티샘플링 퀼리티
    depthDesc.Usage = D3D11_USAGE_DEFAULT;               // 디폴트로 사용
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;                        // cpu 액세스 여부
    depthDesc.MiscFlags = 0;

    result = mDevice->CreateTexture2D(&depthDesc, nullptr, &mDepthStencilTexture);
    if (FAILED(result))
    {
        ASSERT(false, "depth stencil texture 생성 실패");
        return E_FAIL;
    }
    SET_PRIVATE_DATA(mDepthStencilTexture, "Renderer::mDepthStencilTexture");


    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = depthDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    result = mDevice->CreateDepthStencilView(mDepthStencilTexture, &depthStencilViewDesc, &mDepthStencilView);
    if (FAILED(result))
    {
        ASSERT(false, "depth stencil view 생성 실패");
        return E_FAIL;
    }
    SET_PRIVATE_DATA(mDepthStencilView, "Renderer::mDepthStencilView");

    mDeviceContext->OMSetRenderTargets(1, &mBackBufferRTV, mDepthStencilView);


    result = createRasterState();
    if(FAILED(result))
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Renderer::CreateVertexShaderAndInputLayout(
    const WCHAR* const path, D3D11_INPUT_ELEMENT_DESC* const desc, uint32 numDescElements,
    ID3D11VertexShader** const outVertexShader, ID3D11InputLayout** const outInputLayout)
{
    ASSERT(outInputLayout != nullptr, "do not pass nullptr");
    ASSERT(outVertexShader != nullptr, "do not pass nullptr");

    ID3DBlob* blob = nullptr;
    HRESULT result = compileShaderFromFile(path, "main", "vs_5_0", &blob);
    if(FAILED(result))
    {
        ASSERT(false, "failed to compile vertex shader : compileShaderFromFile");
        return E_FAIL;
    }

    result = mDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &(*outVertexShader));
    if (FAILED(result))
    {
        ASSERT(false, "failed to create InputLayout : CreateInputLayout");
        return E_FAIL;
    }

    result = mDevice->CreateInputLayout(desc, numDescElements, blob->GetBufferPointer(), blob->GetBufferSize(), &(*outInputLayout));
    if (FAILED(result))
    {
        ASSERT(false, "failed to create InputLayout : CreateInputLayout");
        return E_FAIL;
    }
    blob->Release();

    return result;
}

HRESULT Renderer::CreateVertexShader(
    const WCHAR* const path, ID3D11VertexShader** const outVertexShader)
{
    ASSERT(outVertexShader != nullptr, "do not pass nullptr");

    ID3DBlob* blob = nullptr;
    HRESULT result = compileShaderFromFile(path, "main", "vs_5_0", &blob);
    if (FAILED(result))
    {
        ASSERT(false, "failed to compile vertex shader : compileShaderFromFile");
        return E_FAIL;
    }

    result = mDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &(*outVertexShader));
    if (FAILED(result))
    {
        ASSERT(false, "failed to create InputLayout : CreateInputLayout");
        return E_FAIL;
    }

    blob->Release();

    return result;
}

HRESULT Renderer::CreatePixelShader(
    const WCHAR* const path, ID3D11PixelShader** const outPixelShader)
{
    ASSERT(outPixelShader != nullptr, "do not pass nullptr");

    ID3DBlob* blob = nullptr;
    // PS_Lighting
    HRESULT result = compileShaderFromFile(path, "main", "ps_5_0", &blob);
    if (FAILED(result))
    {
        ASSERT(false, "failed to compile pixel shader : compileShaderFromFile");
        return E_FAIL;
    }

    result = mDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &(*outPixelShader));
    blob->Release();
    if (FAILED(result))
    {
        return result;
    }

    return result;
}

HRESULT Renderer::CreateTextureResource(
    const WCHAR* fileName, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc,
    ID3D11ShaderResourceView** outShaderResourceView)
{
    ScratchImage image;
    ID3D11Resource* textureResource = nullptr;

    HRESULT result = LoadFromWICFile(fileName, WIC_FLAGS_NONE, nullptr, image);
    if (FAILED(result))
    {
        ASSERT(false, "failed to load imamge file : 이미지 로드 실패");
        goto FAILED;
    }

    result = CreateTexture(mDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &textureResource);
    if (FAILED(result))
    {
        ASSERT(false, "failed to create TextureResource : gTextureResource 생성 실패");
        goto FAILED;
    }

    result = mDevice->CreateShaderResourceView(textureResource, &srvDesc, &(*outShaderResourceView));
    if (FAILED(result))
    {
        ASSERT(false, "failed to create outShaderResourceView : outShaderResourceView 생성 실패");
        goto FAILED;
    }

    result = S_OK;

FAILED:
    image.Release();
    SAFETY_RELEASE(textureResource);

    return result;
    
}

void Renderer::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) const
{
    mDeviceContext->IASetPrimitiveTopology(topology);
}

void Renderer::SetRasterState(eRasterType type)
{
    mDeviceContext->RSSetState(mRasterStates[static_cast<uint32>(type)]);
}

void Renderer::ClearScreenAndDepth()
{
    constexpr float CLEAR_COLOR[] = { 0.4f, 0.6f, 1.0f, 1.0f };
    mDeviceContext->ClearRenderTargetView(mBackBufferRTV, CLEAR_COLOR);
    mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::ClearDepthBuffer()
{
    mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::Present() const
{
    mSwapChain->Present(0, 0);
}

HRESULT Renderer::CheckDeviceLost()
{
    HRESULT result = mDevice->GetDeviceRemovedReason();

    switch (result)
    {
    case S_OK:
        return S_OK;

    case DXGI_ERROR_DEVICE_HUNG:
    case DXGI_ERROR_DEVICE_RESET:

        Cleanup();

        DXGI_SWAP_CHAIN_DESC swapDesc;
        ZeroMemory(&swapDesc, sizeof(swapDesc));
        swapDesc.BufferCount = 1;
        swapDesc.BufferDesc.Width = mWindowWidth;
        swapDesc.BufferDesc.Height = mWindowHeight;
        swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapDesc.OutputWindow = mhWindow;
        swapDesc.SampleDesc.Count = 1;
        swapDesc.SampleDesc.Quality = 0;
        swapDesc.Windowed = TRUE;

        if (FAILED(CreateDeviceAndSetup(swapDesc, mhWindow, mWindowWidth, mWindowHeight, true)))
        {
            SendMessage(mhWindow, WM_DESTROY, 0, 0);
            return E_FAIL;
        }

        return S_OK;
    case DXGI_ERROR_DEVICE_REMOVED:
    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
    case DXGI_ERROR_INVALID_CALL:
    default:
        SendMessage(mhWindow, WM_DESTROY, 0, 0);
        return E_FAIL;
    }
}

void Renderer::Cleanup()
{

    SAFETY_RELEASE(mDevice);
    SAFETY_RELEASE(mDevice);
    SAFETY_RELEASE(DefaultTexture);
    SAFETY_RELEASE(mBackBufferRTV);
    SAFETY_RELEASE(mDepthStencilTexture);
    SAFETY_RELEASE(mDepthStencilView);
    SAFETY_RELEASE(mSwapChain);
    SAFETY_RELEASE(mDeviceContext);
    SAFETY_RELEASE(mDevice);
}

ULONG Renderer::AddRef()
{
    ++mRefCount;
    return mRefCount;
}

ULONG Renderer::Release()
{
    --mRefCount;
    if(mRefCount <= 0)
    {
        delete this;
    }
    return mRefCount;
}

HRESULT Renderer::QueryInterface(const IID& riid, void** ppvObject)
{
    ASSERT(false, "not implements");
    return E_FAIL;
}
