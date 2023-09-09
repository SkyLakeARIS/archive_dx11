// ModelViewerDx11.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "Camera.h"
#include "ModelImporter.h"
#include "ModelViewerDx11.h"
#include "Plane.h"

#include <string>


using namespace DirectX;
#define MAX_LOADSTRING 100

//#define USING_MYINPUT



// 16byte aligned
struct CBCamera
{
    XMMATRIX mView;
};

struct CBChangeOnResize
{
    XMMATRIX mProjection;
};

struct CBChangesEveryFrame
{
    XMMATRIX mWorld;
};

// outline
struct CBOutline
{
    XMMATRIX mWorldViewProjection;
};

struct CBLight
{
    XMFLOAT4 vLightColor;
    XMFLOAT4 vLightDir;
};

// 전역 변수:
HINSTANCE   ghInst;                                // 현재 인스턴스입니다.
HWND        gWnd;
WCHAR       szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR       szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
INT32       gWidth = 1024;
INT32       gHeight = 720;

// global d3d11 objects
ID3D11Device*                   gDevice = nullptr;                   // 본체, 리소스, 개체 생성 해제 기능.
ID3D11DeviceContext*            gDeviceContext = nullptr;            // 흐름(전역적인 현재 상태), 여러 개 사용가능하긴 함., 화면에 그리는 것, 상태바꾸는 기능.
IDXGISwapChain*                 gSwapChain = nullptr;                // 렌더링한 결과를 출력할 대상
ID3D11RenderTargetView*         gRenderTargetView = nullptr;         // 화면에 그려질 버퍼
ID3D11Texture2D*                gDepthStencil = nullptr;
ID3D11DepthStencilView*         gDepthStencilView = nullptr;
ID3D11RasterizerState*          gBasicRasterState = nullptr;

ID3D11VertexShader* gVsOutline = nullptr;
ID3D11PixelShader* gPsOutline = nullptr;

// outline
ID3D11RasterizerState*          gOutlineRasterState = nullptr;


D3D_DRIVER_TYPE                 gDriverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL               gFeatureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11VertexShader*             gVertexShader = nullptr;
ID3D11PixelShader*              gPixelShaderTextureAndLighting = nullptr;

ID3D11SamplerState*             gSamplerAnisotropic = nullptr;
//enum {NUMBER_TEXTURE = 5};
//ID3D11ShaderResourceView*       gShaderResourceView[NUMBER_TEXTURE];

ID3D11InputLayout*              gVertexLayout = nullptr;
//ID3D11Buffer*                   gVertexBuffer = nullptr;
//ID3D11Buffer*                   gIndexBuffer = nullptr;

ID3D11Buffer*                   gCBCamera = nullptr;
ID3D11Buffer*                   gCBChangeOnResize = nullptr;
ID3D11Buffer*                   gCBChangesEveryFrame1 = nullptr;
ID3D11Buffer*                   gCBLight = nullptr;
ID3D11Buffer*                   gCBOutline = nullptr;


// global model
ModelImporter*          gImporter = nullptr;
Model*                  gModel = nullptr;

//size_t                  gVertexCount = 0;
//Vertex*             gVertexList = nullptr;
//size_t                  gIndexListCount = 0;
//unsigned int*           gIndexList = nullptr;

#ifdef USING_MYINPUT
MyInput*                gMyInput = nullptr;

#else
DirectInput*            gDirectInput = nullptr;

#endif

// 여기는 카메라+물체가 가지는 행렬, 굳이 전역으로 뺄 필요없긴 함.
XMMATRIX gMatWorld1;

Camera gCamera(XMVectorSet(0.0f, 10.0f, -15.0f, 0.0f)
    , XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f)
    , XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

// 물체 관련 전역 - 임시
float   gScaleFactor = 1.0f;
//size_t  gIndexOfFocusedVertex;


// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HRESULT             InitializeD3D();
HRESULT             SetupGeometry();
HRESULT             UpdateFrame(float deltaTime);
HRESULT             Render(float deltaTime);
void                Cleanup();


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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


#ifdef _DEBUG
void CheckLiveObjects()
{
    HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
    ASSERT(dxgidebugdll != NULL, "dxgidebug.dll 로드 실패");

    decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));

    IDXGIDebug* debug;

    GetDebugInterface(IID_PPV_ARGS(&debug));

    OutputDebugStringW(L"====================== Direct3D Object ref count 메모리 누수 체크 ===================================\r\n");
    debug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugStringW(L"====================== 반환되지 않은 IUnknown 객체가 있을경우 위에 나타납니다. ======================\r\n");


    debug->Release();
}
#endif

HRESULT LoadTextureFromFileAndCreateResource(ID3D11Device& device, const WCHAR* fileName, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc, ID3D11ShaderResourceView** outShaderResourceView)
{
    ScratchImage image;
    ID3D11Resource* textureResource = nullptr;

    HRESULT result = LoadFromWICFile(fileName, WIC_FLAGS_NONE, nullptr, image);
    if (FAILED(result))
    {
        MessageBox(gWnd, std::to_wstring(result).c_str(), L"이미지 로드 실패", MB_OK);
        ASSERT(false, "이미지 로드 실패");
        goto FAILED;
    }

    result = CreateTexture(gDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &textureResource);
    if (FAILED(result))
    {
        MessageBox(gWnd, std::to_wstring(result).c_str(), L"CreateTexture gTextureResource 생성 실패", MB_OK);
        ASSERT(false, "CreateTexture gTextureResource 생성 실패");
        goto FAILED;
    }


    result = gDevice->CreateShaderResourceView(textureResource, &srvDesc, &(*outShaderResourceView));
    if (FAILED(result))
    {
        ASSERT(false, "outShaderResourceView 생성 실패");
        goto FAILED;
    }

    result = S_OK;

    FAILED:
    image.Release();
    SAFETY_RELEASE(textureResource);

    return result;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MODELVIEWERDX11, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    int programReturn = FALSE;
    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        ASSERT(false, "초기화 실패");
        goto EXIT_PROGRAM;
    }

    if (FAILED(InitializeD3D()))
    {
        ASSERT(false, "d3d초기화 실패");
        goto EXIT_PROGRAM;
    }


    if (FAILED(SetupGeometry()))
    {
        ASSERT(false, "모델데이터 초기화 실패 SetupGeometry");
        goto EXIT_PROGRAM;
    }


    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    Timer::Initialize();

    SetCursorPos(gWidth/(float)2, gHeight/(float)2);

    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
        {
            bool fHandled = false;
#ifdef USING_MYINPUT
            if (!(gMyInput->GetControlMode() & (uint32)eControlFlags::KEYBOARD_MOVEMENT_MODE) && (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
            {
                fHandled = gMyInput->UpdateMouseInput(msg);
            }
            else if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
            {
                fHandled = gMyInput->UpdateKeyboardInput(msg);
            }
#endif
            if (WM_QUIT == msg.message)
            {
                break;
            }

            if (!fHandled)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            Timer::Tick();
#ifndef USING_MYINPUT
            gDirectInput->UpdateInput();
#endif
            UpdateFrame(Timer::GetDeltaTime());
            Render(Timer::GetDeltaTime());
        }
    }


    programReturn = (int)msg.wParam;
    EXIT_PROGRAM:
    Cleanup();
#ifdef _DEBUG
    CheckLiveObjects();
#endif
    return programReturn;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MODELVIEWERDX11));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MODELVIEWERDX11);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    ghInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    gWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        0, 0, gWidth, gHeight, nullptr, nullptr, hInstance, nullptr);

    if (!gWnd)
    {
        return FALSE;
    }

    ShowWindow(gWnd, nCmdShow);
    UpdateWindow(gWnd);

    return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HRESULT InitializeD3D()
{
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

    // 백버퍼와 프론트 버퍼를 스왑하는 방식, 백버퍼에서 프론트로 카피하는 방식 두 개가 존재함.
    DXGI_SWAP_CHAIN_DESC swapDesc;
    ZeroMemory(&swapDesc, sizeof(swapDesc));
    swapDesc.BufferCount = 1;
    swapDesc.BufferDesc.Width = gWidth;
    swapDesc.BufferDesc.Height = gHeight;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = gWnd;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
    swapDesc.Windowed = TRUE;

    UINT32 createDeviceFlag = 0;
#ifdef _DEBUG
    createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result = NULL;
    for (UINT32 driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
    {
        gDriverType = driverTypes[driverTypeIndex];
        result = D3D11CreateDeviceAndSwapChain(nullptr, gDriverType, nullptr, createDeviceFlag, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &swapDesc, &gSwapChain, &gDevice, &gFeatureLevel, &gDeviceContext);
        if (SUCCEEDED(result))
        {
            break;
        }
    }

    if (FAILED(result))
    {
        return E_FAIL;
    }

    SET_PRIVATE_DATA(gDevice, "gDevice");



    // 백버퍼를 얻어와서 렌더타겟으로 설정하는 부분
    ID3D11Texture2D* backBuffer = nullptr;
    // 데스크탑 버전에서는 기본적으로 스왑체인은 하나의 백버퍼를 가지고 있고, uwp는 만들어줘야한다.
    result = gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(result))
    {
        return E_FAIL;
    }
    // view가 붙은것과 아닌것의 차이가 있다.
    // 어떤 리소스가 있고 그것을 사용하기 위한 파생 인터페이스. 보통 읽기전용으로 동작한다.
    // 9과달리 11에서는 어떤 원천 리소스를 가지고 어떤 방식으로 사용하냐에 따라 인터페이스가 다 나뉘어 있다고 한다.
    result = gDevice->CreateRenderTargetView(backBuffer, nullptr, &gRenderTargetView);
    // 백버퍼의 역할은 렌더타겟으로 지정해주는 것이 끝이다.
    // refCount이기 때문에 바로 사라지는건 아니고, gRenderTargetView가 백버퍼를 가지기 때문에 이후에 사라질 것.
    backBuffer->Release();
    if (FAILED(result))
    {
        return E_FAIL;
    }

    // output merger, 그래픽 버퍼에 써넣는 일을 할 때 OM이 붙음.
    gDeviceContext->OMSetRenderTargets(1, &gRenderTargetView, nullptr);

    D3D11_VIEWPORT viewport;
    viewport.Width = (FLOAT)gWidth;
    viewport.Height = (FLOAT)gHeight;
    // 보통 0~1 값으로 지정한다.
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    // 렌더링될 영역을 지정. s가 붙으니까 여러개 지정가능.(모델링 프로그램을 생각)
    // rasterizer stage
    gDeviceContext->RSSetViewports(1, &viewport);

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = gWidth;
    depthDesc.Height = gHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;                      // 멀티샘플링 수
    depthDesc.SampleDesc.Quality = 0;                    // 멀티샘플링 퀼리티
    depthDesc.Usage = D3D11_USAGE_DEFAULT;               // 디폴트로 사용
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;                        // cpu 액세스 여부
    depthDesc.MiscFlags = 0;

    result = gDevice->CreateTexture2D(&depthDesc, nullptr, &gDepthStencil);
    if (FAILED(result))
    {
        ASSERT(false, "depth stencil 생성 실패");
        return E_FAIL;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Format = depthDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    result = gDevice->CreateDepthStencilView(gDepthStencil, &depthStencilViewDesc, &gDepthStencilView);
    if (FAILED(result))
    {
        ASSERT(false, "depth stencil view 생성 실패");
        return E_FAIL;
    }

    gDeviceContext->OMSetRenderTargets(1, &gRenderTargetView, gDepthStencilView);

    // 기본 래스터 스테이트
    D3D11_RASTERIZER_DESC basicRasterDesc;
    ZeroMemory(&basicRasterDesc, sizeof(D3D11_RASTERIZER_DESC));

    basicRasterDesc.CullMode = D3D11_CULL_BACK;
    basicRasterDesc.FillMode = D3D11_FILL_SOLID;
    basicRasterDesc.FrontCounterClockwise = false;
    gDevice->CreateRasterizerState(&basicRasterDesc, &gBasicRasterState);

    // 아웃라인용 래스터 스테이트
    D3D11_RASTERIZER_DESC outlineRasterDesc;
    ZeroMemory(&outlineRasterDesc, sizeof(D3D11_RASTERIZER_DESC));

    outlineRasterDesc.CullMode = D3D11_CULL_FRONT;
    outlineRasterDesc.FillMode = D3D11_FILL_SOLID;
    outlineRasterDesc.FrontCounterClockwise = false;
    gDevice->CreateRasterizerState(&outlineRasterDesc, &gOutlineRasterState);

    //
    // DirectInput/MyInput initialize
    //
#ifdef USING_MYINPUT
    gMyInput = new MyInput;
    if (FAILED(gMyInput->Initialize(&gWnd, gWidth, gHeight)))
    {
        return E_FAIL;
    }
#else
    gDirectInput = new DirectInput;
    if (FAILED(gDirectInput->Initialize(ghInst, gWnd, gWidth, gHeight)))
    {
        return E_FAIL;
    }
#endif

    return S_OK;
}

HRESULT SetupGeometry()
{
    HRESULT result;


    ID3DBlob* vsBlob = nullptr;

    result = CompileShaderFromFile(L"Shaders/VsBasic.hlsl", "main", "vs_5_0", &vsBlob);
    if (FAILED(result))
    {
        MessageBoxA(gWnd, "vertex shader can not be compiled. please check the file", "ERROR", MB_OK);
        return E_FAIL;
    }

    result = gDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &gVertexShader);

    if (FAILED(result))
    {
        vsBlob->Release();
        return E_FAIL;
    }

    // 셰이더에 전달할 정보. 시멘틱.
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }

    };
    UINT numElements = ARRAYSIZE(layout);
    
    result = gDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &gVertexLayout);
    vsBlob->Release();
    if (FAILED(result))
    {
        return E_FAIL;
    }


    //// 보통 IASetVertexBuffer 세팅은 렌더링할 때 사용한다고 한다.
    //// stride는 몇바이트 건너뛰어야 다음 버텍스가 나오는가, offset은 데이터 내부에서 몇번째 바이트에 버텍스 정보가 있는가.
    //// 예제에서는 단순하기 때문에 미리 세팅.
   // gDeviceContext->IASetInputLayout(gVertexLayout);


    // 픽셀 셰이더도 마찬가지로 진행
    // 4.0은 D3D10버전 셰이더
    // 여러개 만들어도 된다. 컴파일 할 때 함수명만 잘 지정해두면. (여러 셰이더 컴파일 해두고, blob만 바꿔서 런타임에 쓰도록 하는것?)
    ID3DBlob* psBlob = nullptr;
    // PS_Lighting
    result = CompileShaderFromFile(L"Shaders/PsBasic.hlsl", "main", "ps_5_0", &psBlob);
    if (FAILED(result))
    {
        MessageBoxA(gWnd, "pixel shader solid can not be compiled. please check the file", "ERROR", MB_OK);
        return E_FAIL;
    }

    result = gDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &gPixelShaderTextureAndLighting);
    psBlob->Release();
    if (FAILED(result))
    {
        return result;
    }

    result = CompileShaderFromFile(L"Shaders/VsOutline.hlsl", "main", "vs_5_0", &vsBlob);
    if (FAILED(result))
    {
        MessageBoxA(gWnd, "vertex shader can not be compiled. please check the file", "ERROR", MB_OK);
        return E_FAIL;
    }

    result = gDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &gVsOutline);

    vsBlob->Release();
    if (FAILED(result))
    {
        return E_FAIL;
    }

    result = CompileShaderFromFile(L"Shaders/PsOutline.hlsl", "main", "ps_5_0", &psBlob);
    if (FAILED(result))
    {
        MessageBoxA(gWnd, "pixel shader solid can not be compiled. please check the file", "ERROR", MB_OK);
        return E_FAIL;
    }

    result = gDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &gPsOutline);
    psBlob->Release();
    if (FAILED(result))
    {
        return result;
    }
    gImporter = new ModelImporter(gDevice);
    gImporter->Initialize();
    Renderer renderer(gDevice, gDeviceContext); // dummy

    gModel = new Model(&renderer, gDevice, gDeviceContext);

    gImporter->LoadFbxModel("/models/unagi.fbx");

    result = gModel->SetupMesh(*gImporter);
    if (FAILED(result))
    {
        return result;
    }

    result = gModel->SetupShader(eShader::BASIC, gVertexShader, gPixelShaderTextureAndLighting, gVertexLayout);
    if (FAILED(result))
    {
        return result;
    }

    //size_t numMesh = gImporter->GetMeshCount();
    //for (size_t meshIndex = 0; meshIndex < numMesh; ++meshIndex)
    //{
    //    OutputDebugStringW(gModel->GetMeshName(meshIndex));
    //    OutputDebugStringW(L"\n");
    //    gVertexCount += gModel->GetVertexCount(meshIndex);
    //}

    //gVertexList = new Vertex[gVertexCount];
    //gModel->UpdateVertexBuffer(gVertexList, gVertexCount, 0);


    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    //bd.ByteWidth = sizeof(Vertex) * gVertexCount;
    //bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    //D3D11_SUBRESOURCE_DATA initData = {};
    //initData.pSysMem = gVertexList;

    //result = gDevice->CreateBuffer(&bd, &initData, &gVertexBuffer);
    //if (FAILED(result))
    //{
    //    ASSERT(false, "버텍스 버퍼 생성 실패 : gVertexBuffer");
    //    return E_FAIL;
    //}

    // 메모리를 아끼려는건지?
    // 내부에서 저장하고 쓰는게 아니라 넘겨준 변수를 그대로 활용하는 듯.
    // 따라서 0도 변수로 전달.


    /*
     *  인덱스 버퍼
     */
    //for (size_t meshIndex = 0; meshIndex < numMesh; ++meshIndex)
    //{
    //    gIndexListCount += gModel->GetIndexListCount(meshIndex);
    //}
    //gIndexList = new unsigned int[gIndexListCount];
    //gModel->UpdateIndexBuffer(gIndexList, gIndexListCount, 0);

    //bd.Usage = D3D11_USAGE_DEFAULT;
    ////bd.ByteWidth = sizeof(WORD) * 36;
    //bd.ByteWidth = sizeof(unsigned int) * gIndexListCount;
    //bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //bd.CPUAccessFlags = 0;

    //initData.pSysMem = indices;
    //initData.pSysMem = gIndexList;

    //result = gDevice->CreateBuffer(&bd, &initData, &gIndexBuffer);
    //if (FAILED(result))
    //{
    //    ASSERT(false, "인덱스 버퍼 생성 실패");
    //    return E_FAIL;
    //}

    //  DXGI_FORMAT_R16_UINT DXGI_FORMAT_R32G32_UINT
    //gDeviceContext->IASetIndexBuffer(gIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    

    gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // 상수 버퍼 생성 : matView
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBCamera);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;

    result = gDevice->CreateBuffer(&bd, nullptr, &gCBCamera);
    if (FAILED(result))
    {
        ASSERT(false, "상수 버퍼 생성 실패 : gCBCamera");
        return E_FAIL;
    }

    // 상수 버퍼 생성 : projectionView
    bd.ByteWidth = sizeof(CBChangeOnResize);
    result = gDevice->CreateBuffer(&bd, nullptr, &gCBChangeOnResize);
    if (FAILED(result))
    {
        ASSERT(false, "상수 버퍼 생성 실패 : gCBChangeOnResize");
        return E_FAIL;
    }

    // 상수 버퍼 생성 : matWorld and color
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    result = gDevice->CreateBuffer(&bd, nullptr, &gCBChangesEveryFrame1);
    if (FAILED(result))
    {
        ASSERT(false, "상수 버퍼 생성 실패 : gCBChangesEveryFrame");
        return E_FAIL;
    }
    // 상수 버퍼 생성 : light
    bd.ByteWidth = sizeof(CBLight);
    result = gDevice->CreateBuffer(&bd, nullptr, &gCBLight);
    if (FAILED(result))
    {
        ASSERT(false, "상수 버퍼 생성 실패 : gCBLight");
        return E_FAIL;
    }

    // 상수 버퍼 생성 : light
    bd.ByteWidth = sizeof(CBOutline);
    result = gDevice->CreateBuffer(&bd, nullptr, &gCBOutline);
    if (FAILED(result))
    {
        ASSERT(false, "상수 버퍼 생성 실패 : CBOutline");
        return E_FAIL;
    }
    
    D3D11_SAMPLER_DESC samplerDesc = {};
    // D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR  D3D11_FILTER_MIN_MAG_MIP_POINT
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; // 아직 정확히는 잘 모름.
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = gDevice->CreateSamplerState(&samplerDesc, &gSamplerAnisotropic);
    if (FAILED(result))
    {
        ASSERT(false, "SamplerState 생성 실패");
        return E_FAIL;
    }

    // 텍스처 입힐 때 다시 활성화
    //// 로드한 텍스처의 리소스를 셰이더 리소스뷰로 전환 생성
    //D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    //ZeroMemory(&srvDesc, sizeof(srvDesc));
    //srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    //srvDesc.Texture2D.MipLevels = 1;
    //srvDesc.Texture2D.MostDetailedMip = 0;

    // Anbi_Body  Anbi_Hair  Anbi_Face AnbiWeapon_CompanyB_Blade_Fire01
    //LoadTextureFromFileAndCreateResource(*gDevice, L"textures/Unagi_Body.png", srvDesc, &gShaderResourceView[0]);
    //LoadTextureFromFileAndCreateResource(*gDevice, L"textures/Unagi_Hair.png", srvDesc, &gShaderResourceView[2]);
    //LoadTextureFromFileAndCreateResource(*gDevice, L"textures/Unagi_Face.png", srvDesc, &gShaderResourceView[1]);
    //LoadTextureFromFileAndCreateResource(*gDevice, L"textures/Unagi_Weapon.png", srvDesc, &gShaderResourceView[3]);


    gMatWorld1 = XMMatrixIdentity();

    // 초점 대상의 버텍스를 월드로 변환 (임시로 render와 동일한 world TM 사용)
  //  gIndexOfFocusedVertex = (int)(gVertexCount / (float)2);
    XMFLOAT3 focusPoint = gModel->GetCenterPoint();
    XMMATRIX matFocus = XMMatrixIdentity()* XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor);
    XMStoreFloat3(&focusPoint, XMVector3TransformCoord(XMLoadFloat3(&focusPoint), matFocus));

    gCamera.ChangeFocus(focusPoint);

    CBChangeOnResize cbChangeOnResize;
    cbChangeOnResize.mProjection = XMMatrixTranspose(gCamera.GetProjectionMatrix());
    gDeviceContext->UpdateSubresource(gCBChangeOnResize, 0, nullptr, &cbChangeOnResize, 0, 0);


    srand(time(NULL));
    return S_OK;
}

HRESULT UpdateFrame(float deltaTime)
{

    constexpr float speed = 100.0f;

#ifdef USING_MYINPUT
    /*
     *  myinput ver
     */
    int keyboardArrSize = 0;
    bool gKeyboard[256];
    gMyInput->GetKeyboardPressed(gKeyboard, &keyboardArrSize);
    if(gMyInput->GetControlMode() & (uint32)eControlFlags::KEYBOARD_MOVEMENT_MODE)
    {
        if (gKeyboard['W'] || gKeyboard['w'])
        {
            gCamera.RotateAxis(0.0f, XMConvertToRadians(-speed * deltaTime));
        }
        if (gKeyboard['S'] || gKeyboard['s'])
        {
            gCamera.RotateAxis(0.0f, XMConvertToRadians(speed * deltaTime));
        }
        if (gKeyboard['A'] || gKeyboard['a'])
        {
            gCamera.RotateAxis(XMConvertToRadians(-speed * deltaTime), 0.0f);
        }
        if (gKeyboard['D'] || gKeyboard['d'])
        {
            gCamera.RotateAxis(XMConvertToRadians(speed * deltaTime), 0.0f);
        }
    }

    if (gKeyboard['Q'] || gKeyboard['q'])
    {
        gCamera.AddRadiusSphere(speed * deltaTime);
    }
    if (gKeyboard['E'] || gKeyboard['e'])
    {
        gCamera.AddRadiusSphere(-speed * deltaTime);
    }

    // 물체 스케일링
    if (gKeyboard['R'] || gKeyboard['r'])
    {
        gScaleFactor += 0.1f;
        if (gScaleFactor > 15.0f)
        {
            gScaleFactor = 15.0f;
        }
    }

    if (gKeyboard['F'] || gKeyboard['f'])
    {
        gScaleFactor -= 0.1f;
        if (gScaleFactor < 0.1f)
        {
            gScaleFactor = 0.1f;
        }
    }

    if (gKeyboard['C'] || gKeyboard['c'])
    {
        gMyInput->SetControlMode((uint32)eControlFlags::KEYBOARD_MOVEMENT_MODE);
    }

    if (gKeyboard[VK_ESCAPE])
    {
        SendMessage(gWnd, WM_DESTROY, 0, 0);
    }

    /*
     *  카메라 거리 메서드랑 input 클래스 한번 점검해서 라디안 모드로 변경 그리고 그에맞게 시그니처 변경
     *  그리고 키 컨트롤 모드 테스트하기
     *  메인 로직 점검 후, 불필요 코드 과감히 삭제
     */

    ///*
    // *  마우스 움직임은 매끄럽게 처리안되고 있음.
    // */
    int mouseX = 0;
    int mouseY = 0;
    gMyInput->GetMouseDeltaPosition(mouseX, mouseY);

    gCamera.RotateAxis(XMConvertToRadians(mouseX * deltaTime), XMConvertToRadians(mouseY * deltaTime));


#else
    /*
     *  direct input ver
     */

    int keyboardArrSize = 0;
    unsigned char* gKeyboard = gDirectInput->GetKeyboardPress();


    if(!(gDirectInput->GetControlMode() & (uint32)eControlFlags::KEYBOARD_MOVEMENT_MODE))
    {
        int mouseX = 0;
        int mouseY = 0;
        gDirectInput->GetMouseDeltaPosition(mouseX, mouseY);
        gCamera.RotateAxis(XMConvertToRadians(mouseX * deltaTime), XMConvertToRadians(mouseY * deltaTime));
    }
    else
    {
        if (gKeyboard[DIK_W] & 0x80)
        {
            gCamera.RotateAxis( 0.0f, XMConvertToRadians(-speed * deltaTime));
        }

        if (gKeyboard[DIK_S] & 0x80)
        {
            gCamera.RotateAxis(0.0f, XMConvertToRadians(speed * deltaTime));
        }
        if (gKeyboard[DIK_A] & 0x80)
        {
            gCamera.RotateAxis(XMConvertToRadians(-speed * deltaTime), 0.0f);
        }

        if (gKeyboard[DIK_D] & 0x80)
        {
            gCamera.RotateAxis(XMConvertToRadians(speed * deltaTime), 0.0f);
        }       
    }

    // 마우스 휠 처리 이전에 임시용.
    // 카메라와 물체간의 거리 조절(구체 크기 확대/축소)
    if (gKeyboard[DIK_Q] & 0x80)
    {
        gCamera.AddRadiusSphere(speed * deltaTime);
    }

    if (gKeyboard[DIK_E] & 0x80)
    {
        gCamera.AddRadiusSphere(-speed * deltaTime);
    }

    // 물체 스케일링
    if (gKeyboard[DIK_R] & 0x80)
    {
        gScaleFactor += 0.1f * speed * deltaTime;
        if(gScaleFactor > 15.0f)
        {
            gScaleFactor = 15.0f;
        }
        // 변경된 사이즈에 의해 달라진 초점 위치를 다시 계산한 뒤,
        // 월드공간으로 변환하여 카메라에 전달.
        XMFLOAT3 newFocus;
        XMFLOAT3 center = gModel->GetCenterPoint();

        XMVECTOR lookAt = XMLoadFloat3(&center);
        XMMATRIX matFocus = XMMatrixIdentity() * XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor);
        XMStoreFloat3(&newFocus, XMVector3TransformCoord(lookAt, matFocus));

        gCamera.ChangeFocus(newFocus);
    }

    if (gKeyboard[DIK_F] & 0x80)
    {
        gScaleFactor -= 0.1f * speed * deltaTime;
        if (gScaleFactor < 0.2f)
        {
            gScaleFactor = 0.2f;
        }
        XMFLOAT3 newFocus;
        XMFLOAT3 center = gModel->GetCenterPoint();

        XMVECTOR lookAt = XMLoadFloat3(&center);
        XMMATRIX matFocus = XMMatrixIdentity() * XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor);
        XMStoreFloat3(&newFocus, XMVector3TransformCoord(lookAt, matFocus));

        gCamera.ChangeFocus(newFocus);
    }

    // 키보드<-> 마우스 조작 전환
    if (gKeyboard[DIK_C] & 0x80)
    {
        gDirectInput->SetControlMode((uint32)eControlFlags::KEYBOARD_MOVEMENT_MODE);
    }

    //// 초점 정점 변경
    //if (gKeyboard[DIK_H] & 0x80)
    //{
    //    //
    //    // 랜덤 버텍스 말고, Model에서 최대/최소 y값의 중간값으로 설정.
    //    //
    //    gIndexOfFocusedVertex = rand()%gVertexCount;
    //    XMFLOAT3 focusPoint = gVertexList[gIndexOfFocusedVertex].Position;
    //    XMMATRIX matFocus = XMMatrixIdentity() * XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor);
    //    XMStoreFloat3(&focusPoint, XMVector3TransformCoord(XMLoadFloat3(&focusPoint), matFocus));

    //    gCamera.ChangeFocus(focusPoint);
    //}


    if (gKeyboard[DIK_ESCAPE] & 0x80)
    {
        SendMessage(gWnd, WM_DESTROY, 0, 0);
    }


#endif

    return S_OK;
}

HRESULT Render(float deltaTime)
{
    
    float clearToSkyColor[] = { 0.4f, 0.6f, 1.0f, 1.0f };
    gDeviceContext->ClearRenderTargetView(gRenderTargetView, clearToSkyColor);
    gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


    // outline
    //gDeviceContext->RSSetState(gOutlineRasterState);

    //gDeviceContext->VSSetShader(gVsOutline, nullptr, 0);
    //gDeviceContext->PSSetShader(gPsOutline, nullptr, 0);

    //gDeviceContext->VSSetConstantBuffers(0, 1, &gCBOutline);
    //
    //// arbit fbx model
    //gMatWorld1 = XMMatrixIdentity() * XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor) * gCamera.GetViewMatrix() * gCamera.GetProjectionMatrix();

    //CBOutline cbOutline;
    //cbOutline.mWorldViewProjection = XMMatrixTranspose(gMatWorld1);
    //gDeviceContext->UpdateSubresource(gCBOutline, 0, nullptr, &cbOutline, 0, 0);

    //gModel->Draw();


    //
    // render front
    //

   //gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    gDeviceContext->RSSetState(gBasicRasterState);

    //gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
   //gDeviceContext->PSSetShader(gPixelShaderTextureAndLighting, nullptr, 0);

    gDeviceContext->VSSetConstantBuffers(0, 1, &gCBCamera);
    gDeviceContext->VSSetConstantBuffers(1, 1, &gCBChangeOnResize);
    gDeviceContext->VSSetConstantBuffers(2, 1, &gCBChangesEveryFrame1);

    gDeviceContext->PSSetConstantBuffers(2, 1, &gCBChangesEveryFrame1);
    gDeviceContext->PSSetConstantBuffers(3, 1, &gCBLight);

    gDeviceContext->PSSetSamplers(0, 1, &gSamplerAnisotropic);


    // Setup our lighting parameters
    static XMFLOAT4 lightDirs(0.0f, 0.577f, -0.550f, 1.0f);
    XMFLOAT4 lightColor(0.1f, 0.1f, 0.1f, 1.0f);

    XMMATRIX matRotateLight = XMMatrixRotationY(deltaTime);
    XMVECTOR vLightDir = XMLoadFloat4(&lightDirs);
    vLightDir = XMVector3Transform(vLightDir, matRotateLight);

    XMStoreFloat4(&lightDirs, vLightDir);

    gMatWorld1 = XMMatrixIdentity() * XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor);

    CBChangesEveryFrame cbChangesEveryFrame;
    cbChangesEveryFrame.mWorld = XMMatrixTranspose(gMatWorld1);
    gDeviceContext->UpdateSubresource(gCBChangesEveryFrame1, 0, nullptr, &cbChangesEveryFrame, 0, 0);

    CBCamera camera;
    camera.mView = XMMatrixTranspose(gCamera.GetViewMatrix());
    gDeviceContext->UpdateSubresource(gCBCamera, 0, nullptr, &camera, 0, 0);

    CBChangeOnResize projection;
    projection.mProjection = XMMatrixTranspose(gCamera.GetProjectionMatrix());
    gDeviceContext->UpdateSubresource(gCBChangeOnResize, 0, nullptr, &projection, 0, 0);

    CBLight cbLight;
    cbLight.vLightColor = lightColor;
    cbLight.vLightDir = lightDirs;

    gDeviceContext->UpdateSubresource(gCBLight, 0, nullptr, &cbLight, 0, 0);

    // 리소스뷰를 어떻게 괜찮은 방법으로 처리할 방법을 검색하기
    gModel->Draw();


    gSwapChain->Present(0, 0);
    //디바이스 로스트 처리
    // dx11부터는 처리가 사실상 필요없다는 정보가 있음.
    HRESULT result = gDevice->GetDeviceRemovedReason();

    switch (result)
    {
    case S_OK:
        return S_OK;

    case DXGI_ERROR_DEVICE_HUNG:
    case DXGI_ERROR_DEVICE_RESET:

        Cleanup();

        if (FAILED(InitializeD3D()))
        {
            SendMessage(gWnd, WM_DESTROY, 0, 0);
            return E_FAIL;
        }

        if (FAILED(SetupGeometry()))
        {
            SendMessage(gWnd, WM_DESTROY, 0, 0);
            return E_FAIL;
        }

        return S_OK;
    case DXGI_ERROR_DEVICE_REMOVED:
    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
    case DXGI_ERROR_INVALID_CALL:
    default:
        SendMessage(gWnd, WM_DESTROY, 0, 0);
        return E_FAIL;
    }
}

void Cleanup()
{
#ifdef USING_MYINPUT
    gMyInput->Release();
    delete gMyInput;
    gMyInput = nullptr;
#else
    gDirectInput->Release();
    delete gDirectInput;
    gDirectInput = nullptr;
#endif

    gImporter->Release();
    delete gImporter;

    if (gDeviceContext)
    {
        gDeviceContext->ClearState();
    }
    //delete[] gVertexList;
    //delete[] gIndexList;

    delete gModel;

    SAFETY_RELEASE(gSamplerAnisotropic);


    //for (size_t textureIndex = 0; textureIndex < NUMBER_TEXTURE; ++textureIndex)
    //{
    //    SAFETY_RELEASE(gShaderResourceView[textureIndex]);
    //}

    // outline
    SAFETY_RELEASE(gVsOutline); // shaders
    SAFETY_RELEASE(gPsOutline);
    SAFETY_RELEASE(gBasicRasterState);
    SAFETY_RELEASE(gOutlineRasterState);

    SAFETY_RELEASE(gCBCamera);
    SAFETY_RELEASE(gCBChangeOnResize);
    SAFETY_RELEASE(gCBChangesEveryFrame1);
    SAFETY_RELEASE(gCBLight);
    SAFETY_RELEASE(gCBOutline);

    //SAFETY_RELEASE(gIndexBuffer);
    //SAFETY_RELEASE(gVertexBuffer);
    SAFETY_RELEASE(gVertexLayout);
    SAFETY_RELEASE(gPixelShaderTextureAndLighting);
    SAFETY_RELEASE(gVertexShader);
    SAFETY_RELEASE(gDepthStencil);
    SAFETY_RELEASE(gDepthStencilView);
    SAFETY_RELEASE(gRenderTargetView);
    SAFETY_RELEASE(gSwapChain);
    SAFETY_RELEASE(gDeviceContext);
    SAFETY_RELEASE(gDevice);
}