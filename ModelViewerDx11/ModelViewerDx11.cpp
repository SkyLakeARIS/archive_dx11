// ModelViewerDx11.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "Camera.h"
#include "ModelImporter.h"
#include "ModelViewerDx11.h"
#include "Plane.h"

#include <string>


using namespace DirectX;
#define MAX_LOADSTRING 100



// 전역 변수:
HINSTANCE   ghInst;                                // 현재 인스턴스입니다.
HWND        gWnd;
WCHAR       szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR       szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
INT32       gWidth = 1024;
INT32       gHeight = 720;

// global d3d11 objects
//ID3D11Device*                   gDevice = nullptr;                   // 본체, 리소스, 개체 생성 해제 기능.
//ID3D11DeviceContext*            gDeviceContext = nullptr;            // 흐름(전역적인 현재 상태), 여러 개 사용가능하긴 함., 화면에 그리는 것, 상태바꾸는 기능.
//IDXGISwapChain*                 gSwapChain = nullptr;                // 렌더링한 결과를 출력할 대상
//ID3D11RenderTargetView*         gRenderTargetView = nullptr;         // 화면에 그려질 버퍼
//ID3D11Texture2D*                gDepthStencil = nullptr;
//ID3D11DepthStencilView*         gDepthStencilView = nullptr;
//ID3D11RasterizerState*          gBasicRasterState = nullptr;

ID3D11VertexShader* gVsOutline = nullptr;
ID3D11PixelShader* gPsOutline = nullptr;


D3D_DRIVER_TYPE                 gDriverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL               gFeatureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11VertexShader*             gVertexShader = nullptr;
ID3D11PixelShader*              gPixelShaderTextureAndLighting = nullptr;

ID3D11InputLayout*              gVertexLayout = nullptr;


// global model
ModelImporter*          gImporter = nullptr;
Model*                  gModel = nullptr;

#ifdef USING_MYINPUT
MyInput*                gMyInput = nullptr;

#else
DirectInput*            gDirectInput = nullptr;

#endif


Camera* gCamera = nullptr;


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
    Renderer::CheckLiveObjects();

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

    if (FAILED(Renderer::GetInstance()->CreateDeviceAndSetup(swapDesc, gWnd, gHeight, gWidth, true)))
    {
        return E_FAIL;
    }

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


    //ID3DBlob* vsBlob = nullptr;

  //  result = CompileShaderFromFile(L"Shaders/VsBasic.hlsl", "main", "vs_5_0", &vsBlob);
  //  if (FAILED(result))
 //   {
   //     MessageBoxA(gWnd, "vertex shader can not be compiled. please check the file", "ERROR", MB_OK);
   //     return E_FAIL;
 //   }

  //  result = gDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &);

    //if (FAILED(result))
    //{
    //    vsBlob->Release();
    //    return E_FAIL;
    //}

    // 셰이더에 전달할 정보. 시멘틱.
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }

    };
  //  UINT numElements = ARRAYSIZE(layout);

    result = Renderer::GetInstance()->CreateVertexShader(L"Shaders/VsBasic.hlsl", layout, ARRAYSIZE(layout), &gVertexShader, &gVertexLayout);
  //  result = gDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &);
  //  vsBlob->Release();
    if (FAILED(result))
    {
        return E_FAIL;
    }


    // 픽셀 셰이더도 마찬가지로 진행
    // 4.0은 D3D10버전 셰이더
    // 여러개 만들어도 된다. 컴파일 할 때 함수명만 잘 지정해두면. (여러 셰이더 컴파일 해두고, blob만 바꿔서 런타임에 쓰도록 하는것?)
    //ID3DBlob* psBlob = nullptr;
    //// PS_Lighting
    //result = CompileShaderFromFile(, "main", "ps_5_0", &psBlob);
    //if (FAILED(result))
    //{
    //    MessageBoxA(gWnd, "pixel shader solid can not be compiled. please check the file", "ERROR", MB_OK);
    //    return E_FAIL;
    //}

    result = Renderer::GetInstance()->CreatePixelShader(L"Shaders/PsBasic.hlsl", &gPixelShaderTextureAndLighting);
   //     gDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &gPixelShaderTextureAndLighting);
   // psBlob->Release();
    if (FAILED(result))
    {
        return result;
    }

    

    //result = CompileShaderFromFile(L"Shaders/VsOutline.hlsl", "main", "vs_5_0", &vsBlob);
    //if (FAILED(result))
    //{
    //    MessageBoxA(gWnd, "vertex shader can not be compiled. please check the file", "ERROR", MB_OK);
    //    return E_FAIL;
    //}

    //result = gDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &gVsOutline);

    //vsBlob->Release();
    //if (FAILED(result))
    //{
    //    return E_FAIL;
    //}

    //result = CompileShaderFromFile(L"Shaders/PsOutline.hlsl", "main", "ps_5_0", &psBlob);
    //if (FAILED(result))
    //{
    //    MessageBoxA(gWnd, "pixel shader solid can not be compiled. please check the file", "ERROR", MB_OK);
    //    return E_FAIL;
    //}

    //result = gDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &gPsOutline);
    //psBlob->Release();
    //if (FAILED(result))
    //{
    //    return result;
    //}


    gCamera = new Camera(gWidth, gHeight
        , XMVectorSet(0.0f, 10.0f, -15.0f, 0.0f)
        , XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f)
        , XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));


    gImporter = new ModelImporter(Renderer::GetInstance()->GetDevice());
    gImporter->Initialize();

    gModel = new Model(Renderer::GetInstance(), gCamera);

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

    //gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Renderer::GetInstance()->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gCamera->ChangeFocus(gModel->GetCenterPoint());

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
        gCamera->RotateAxis(XMConvertToRadians(mouseX * deltaTime), XMConvertToRadians(mouseY * deltaTime));
    }
    else
    {
        if (gKeyboard[DIK_W] & 0x80)
        {
            gCamera->RotateAxis( 0.0f, XMConvertToRadians(-speed * deltaTime));
        }

        if (gKeyboard[DIK_S] & 0x80)
        {
            gCamera->RotateAxis(0.0f, XMConvertToRadians(speed * deltaTime));
        }
        if (gKeyboard[DIK_A] & 0x80)
        {
            gCamera->RotateAxis(XMConvertToRadians(-speed * deltaTime), 0.0f);
        }

        if (gKeyboard[DIK_D] & 0x80)
        {
            gCamera->RotateAxis(XMConvertToRadians(speed * deltaTime), 0.0f);
        }       
    }

    // 마우스 휠 처리 이전에 임시용.
    // 카메라와 물체간의 거리 조절(구체 크기 확대/축소)
    if (gKeyboard[DIK_Q] & 0x80)
    {
        gCamera->AddRadiusSphere(speed * deltaTime);
    }

    if (gKeyboard[DIK_E] & 0x80)
    {
        gCamera->AddRadiusSphere(-speed * deltaTime);
    }

    //// 물체 스케일링
    //if (gKeyboard[DIK_R] & 0x80)
    //{
    //    gScaleFactor += 0.1f * speed * deltaTime;
    //    if(gScaleFactor > 15.0f)
    //    {
    //        gScaleFactor = 15.0f;
    //    }
    //    // 변경된 사이즈에 의해 달라진 초점 위치를 다시 계산한 뒤,
    //    // 월드공간으로 변환하여 카메라에 전달.
    //    XMFLOAT3 newFocus;
    //    XMFLOAT3 center = gModel->GetCenterPoint();

    //    XMVECTOR lookAt = XMLoadFloat3(&center);
    //    XMMATRIX matFocus = XMMatrixIdentity() * XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor);
    //    XMStoreFloat3(&newFocus, XMVector3TransformCoord(lookAt, matFocus));

    //    gCamera->ChangeFocus(newFocus);
    //}

    //if (gKeyboard[DIK_F] & 0x80)
    //{
    //    gScaleFactor -= 0.1f * speed * deltaTime;
    //    if (gScaleFactor < 0.2f)
    //    {
    //        gScaleFactor = 0.2f;
    //    }
    //    XMFLOAT3 newFocus;
    //    XMFLOAT3 center = gModel->GetCenterPoint();

    //    XMVECTOR lookAt = XMLoadFloat3(&center);
    //    XMMATRIX matFocus = XMMatrixIdentity() * XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor);
    //    XMStoreFloat3(&newFocus, XMVector3TransformCoord(lookAt, matFocus));

    //    gCamera->ChangeFocus(newFocus);
    //}

    // 키보드<-> 마우스 조작 전환
    static bool bPressKey = false;
    if (!(gKeyboard[DIK_C] & 0x80) && bPressKey)
    {
        gDirectInput->SetControlMode((uint32)eControlFlags::KEYBOARD_MOVEMENT_MODE);
    }
    bPressKey = gKeyboard[DIK_C] & 0x80;

    if (gKeyboard[DIK_ESCAPE] & 0x80)
    {
        SendMessage(gWnd, WM_DESTROY, 0, 0);
    }


#endif

    return S_OK;
}

HRESULT Render(float deltaTime)
{

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

//    gDeviceContext->RSSetState(gBasicRasterState);

    //gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
   //gDeviceContext->PSSetShader(gPixelShaderTextureAndLighting, nullptr, 0);

    //gDeviceContext->VSSetConstantBuffers(0, 1, &gCBCamera);
    //gDeviceContext->VSSetConstantBuffers(1, 1, &gCBChangeOnResize);
    //gDeviceContext->VSSetConstantBuffers(2, 1, &gCBChangesEveryFrame1);

    //gDeviceContext->PSSetConstantBuffers(2, 1, &gCBChangesEveryFrame1);
    //gDeviceContext->PSSetConstantBuffers(3, 1, &gCBLight);

    //gDeviceContext->PSSetSamplers(0, 1, &gSamplerAnisotropic);


    //// Setup our lighting parameters
    //static XMFLOAT4 lightDirs(0.0f, 0.577f, -0.550f, 1.0f);
    //XMFLOAT4 lightColor(0.1f, 0.1f, 0.1f, 1.0f);

    //XMMATRIX matRotateLight = XMMatrixRotationY(deltaTime);
    //XMVECTOR vLightDir = XMLoadFloat4(&lightDirs);
    //vLightDir = XMVector3Transform(vLightDir, matRotateLight);

    //XMStoreFloat4(&lightDirs, vLightDir);

    //gMatWorld1 = XMMatrixIdentity() * XMMatrixScaling(gScaleFactor, gScaleFactor, gScaleFactor);

    //CBChangesEveryFrame cbChangesEveryFrame;
    //cbChangesEveryFrame.mWorld = XMMatrixTranspose(gMatWorld1);
    //gDeviceContext->UpdateSubresource(gCBChangesEveryFrame1, 0, nullptr, &cbChangesEveryFrame, 0, 0);

    //CBCamera camera;
    //camera.mView = XMMatrixTranspose(gCamera->GetViewMatrix());
    //gDeviceContext->UpdateSubresource(gCBCamera, 0, nullptr, &camera, 0, 0);

    //CBChangeOnResize projection;
    //projection.mProjection = XMMatrixTranspose(gCamera->GetProjectionMatrix());
    //gDeviceContext->UpdateSubresource(gCBChangeOnResize, 0, nullptr, &projection, 0, 0);

    //CBLight cbLight;
    //cbLight.vLightColor = lightColor;
    //cbLight.vLightDir = lightDirs;

    //gDeviceContext->UpdateSubresource(gCBLight, 0, nullptr, &cbLight, 0, 0);
    Renderer::GetInstance()->ClearScreen();

    // 리소스뷰를 어떻게 괜찮은 방법으로 처리할 방법을 검색하기
    gModel->Draw();

    Renderer::GetInstance()->Present();
    //디바이스 로스트 처리
    // dx11부터는 처리가 사실상 필요없다는 정보가 있음.
    return S_OK;
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

    //if (gDeviceContext)
    //{
    //    gDeviceContext->ClearState();
    //}

    delete gModel;
    delete gCamera;
    // outline
    SAFETY_RELEASE(gVsOutline); // shaders
    SAFETY_RELEASE(gPsOutline);

    SAFETY_RELEASE(gVertexLayout);
    SAFETY_RELEASE(gPixelShaderTextureAndLighting);
    SAFETY_RELEASE(gVertexShader);
    // device가 가장 마지막에 해제되도록.
    Renderer::GetInstance()->Release();

    //SAFETY_RELEASE(gDepthStencil);
    //SAFETY_RELEASE(gDepthStencilView);
    //SAFETY_RELEASE(gRenderTargetView);
    //SAFETY_RELEASE(gSwapChain);
    //SAFETY_RELEASE(gDeviceContext);
    //SAFETY_RELEASE(gDevice);
}