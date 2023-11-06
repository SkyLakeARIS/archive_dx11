// ModelViewerDx11.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "Camera.h"
#include "ModelImporter.h"
#include "ModelViewerDx11.h"
#include "Plane.h"

#include <string>

#include "Sky.h"


using namespace DirectX;
#define MAX_LOADSTRING 100


// 전역 변수:
WCHAR       szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR       szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

ModelImporter*          gImporter = nullptr;
Model*                  gCharacter = nullptr;
Camera* gCamera = nullptr;
Sky* gSkybox = nullptr;

#ifdef USING_MYINPUT
MyInput*                gMyInput = nullptr;

#else
DirectInput*            gDirectInput = nullptr;

#endif




// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

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

    HRESULT result = S_OK;
    int programReturn = FALSE;
    // 애플리케이션 초기화를 수행합니다:

    const int WINDOW_WIDTH = 1024;
    const int WINDOW_HEIGHT = 720;
    const HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        goto EXIT_PROGRAM;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    // 백버퍼와 프론트 버퍼를 스왑하는 방식, 백버퍼에서 프론트로 카피하는 방식 두 개가 존재함.
    DXGI_SWAP_CHAIN_DESC swapDesc;
    ZeroMemory(&swapDesc, sizeof(swapDesc));
    swapDesc.BufferCount = 1;
    swapDesc.BufferDesc.Width = WINDOW_WIDTH;
    swapDesc.BufferDesc.Height = WINDOW_HEIGHT;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hWnd;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
    swapDesc.Windowed = TRUE;

    result = Renderer::GetInstance()->CreateDeviceAndSetup(swapDesc, hWnd, WINDOW_HEIGHT, WINDOW_WIDTH, true);
    if (FAILED(result))
    {
        ASSERT(false, "모델데이터 초기화 실패 SetupGeometry");

        goto EXIT_PROGRAM;
    }

    //
    // DirectInput/MyInput initialize
    //
#ifdef USING_MYINPUT
    gMyInput = new MyInput;
    result = gMyInput->Initialize(&gWnd, gWidth, gHeight);
    if (FAILED(result))
    {
        goto EXIT_PROGRAM;
    }
#else
    gDirectInput = new DirectInput(hInstance, hWnd, WINDOW_WIDTH, WINDOW_HEIGHT);
    result = gDirectInput->Initialize();
    if (FAILED(result))
    {
        ASSERT(false, "모델데이터 초기화 실패 SetupGeometry");
        goto EXIT_PROGRAM;
    }
#endif

    gCamera = new Camera(
        XMVectorSet(0.0f, 10.0f, -15.0f, 0.0f)
        , XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f)
        , XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));


    gImporter = new ModelImporter(Renderer::GetInstance()->GetDevice());
    gImporter->Initialize();

    gCharacter = new Model(Renderer::GetInstance(), gCamera);

    gImporter->LoadFbxModel("/models/unagi.fbx");

    gSkybox = new Sky(*Renderer::GetInstance(), *gCamera);
    gSkybox->Initialize(10, 10);

    result = gCharacter->SetupMesh(*gImporter);
    if (FAILED(result))
    {
        ASSERT(false, "gCharacter::SetupMesh 모델데이터 혹은 D3D개체 초기화 실패 _ could not initialize mesh or d3d obj");
        goto EXIT_PROGRAM;
    }

    result = gCharacter->SetupShaderFromRenderer();
    //gCharacter->SetupShader(eShader::BASIC, gVertexShader, gPixelShaderTextureAndLighting, gVertexLayout);
    if (FAILED(result))
    {
        ASSERT(false, "gCharacter::SetupShaderFromRenderer 초기화 실패 -failed to create shader");
        goto EXIT_PROGRAM;
    }

    Renderer::GetInstance()->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gCamera->ChangeFocus(gCharacter->GetCenterPoint());


    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    Timer::Initialize();


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

HRESULT SetupGeometry()
{


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

    // 키보드<-> 마우스 조작 전환
    static bool bPressKey = false;
    if (!(gKeyboard[DIK_C] & 0x80) && bPressKey)
    {
        gDirectInput->SetControlMode((uint32)eControlFlags::KEYBOARD_MOVEMENT_MODE);
    }
    bPressKey = gKeyboard[DIK_C] & 0x80;

    static bool bPressHKey = false;
    static bool bHightlight = false;
    if (!(gKeyboard[DIK_H] & 0x80) && bPressHKey)
    {
        bHightlight = bHightlight ? false : true;
        gCharacter->SetHighlight(bHightlight);
    }
    bPressHKey = gKeyboard[DIK_H] & 0x80;

    if (gKeyboard[DIK_ESCAPE] & 0x80)
    {
        SendMessage(Renderer::GetInstance()->GetWindowHandle(), WM_DESTROY, 0, 0);
    }


#endif

    return S_OK;
}

HRESULT Render(float deltaTime)
{

    Renderer::GetInstance()->ClearScreenAndDepth();

    // 리소스뷰를 어떻게 괜찮은 방법으로 처리할 방법을 검색하기
    gSkybox->Draw();
    gCharacter->Draw();

    Renderer::GetInstance()->Present();

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

 //   gImporter->Release();
    delete gSkybox;
    delete gImporter;

    delete gCharacter;
    delete gCamera;
 
    // device가 가장 마지막에 해제되도록.
    Renderer::GetInstance()->Release();
}