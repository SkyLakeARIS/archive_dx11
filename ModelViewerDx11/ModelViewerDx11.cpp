﻿// ModelViewerDx11.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "Camera.h"
#include "ModelImporter.h"
#include "ModelViewerDx11.h"
#include "Plane.h"

#include <string>

#include "Light.h"
#include "LightManager.h"
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
Light* gLight = nullptr;
Plane* gPlane = nullptr;

DirectInput*            gDirectInput = nullptr;



// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HRESULT             SetupGeometry();
HRESULT             UpdateFrame(float deltaTime);
HRESULT             Render(float deltaTime);
void                Cleanup();


void preprocess(float deltaTime);

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


    result = Renderer::GetInstance()->PrepareRender();
    if (FAILED(result))
    {
        ASSERT(false, "Fail to initialize shaders");

        goto EXIT_PROGRAM;
    }

    //
    // DirectInput initialize
    //
    gDirectInput = new DirectInput(hInstance, hWnd, WINDOW_WIDTH, WINDOW_HEIGHT);
    result = gDirectInput->Initialize();
    if (FAILED(result))
    {
        ASSERT(false, "모델데이터 초기화 실패 SetupGeometry");
        goto EXIT_PROGRAM;
    }

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


    Renderer::GetInstance()->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    LightManager::GetInstance();
    gCamera->ChangeFocus(gCharacter->GetCenterPoint());
    // MEMO Light 위치값 막 바꾸면 안됨. 그림자 제대로 안그려질 수 있음. 나중에 개선해야 할 항목 중 하나(cascade)
    gLight = new Light(XMFLOAT3(0.0f, 50.0f, 70.0f), gCharacter->GetCenterPoint(), XMFLOAT3(1.0f, 1.0f, 1.0f));
    gLight->Initialize();
    gCharacter->SetLight(gLight);

    // debug quad
    // 깊이 텍스쳐 확인용
    gPlane = new Plane();
    
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    Timer::Initialize();
    {
        constexpr float FPS_UPDATE = 25.0f;
        constexpr float FPS_RENDER = 120.0f;
        constexpr float UpdateInterval = 1000.0f / FPS_UPDATE;
        constexpr float RenderInterval = 1000.0f / FPS_RENDER;
        float nextUpdateTime = 0.0f;
        float nextUpdateTimeRender = 0.0f;
        float nextUpdateFPS = 0.0f;
        float sumUpdate = 0.0f;
        float sumRender = 0.0f;

        uint8_t updateFPS = 0;
        uint8_t renderFPS = 0;
        while (true)
        {
            float start = Timer::GetNowMS();

            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
            {
                bool fHandled = false;

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
                gDirectInput->UpdateInput();

                while(sumUpdate >= nextUpdateTime)
                {
                    // MEMO 위의 키입력을 토대로
                    // MEMO 아래에서 반영하여 물체에 갱신
                    // 일단 키 인풋에 대한 처리 필요 : 카메라, 캐릭터(지금은 아니지만)
                    // 그외, 앱에서 처리하는 키 입력은 게임의 업데이트와는 무관. 즉시처리해도 됨.
                    // 그렇다면 UpdateInput은 업데이트만하고, UpdateFrame에서 다시 read한다면?
                    // MEMO : 일단, 현 상황에서 필요한 마우스 인풋관련은 처리 완료.
                    sumUpdate -= UpdateInterval;
                    Timer::Tick();

                    UpdateFrame(Timer::GetDeltaTime());

                    ++updateFPS;
                }

               if(sumRender >= nextUpdateTimeRender)
                {

                    nextUpdateTimeRender = sumRender + RenderInterval;

                    preprocess(sumUpdate/ nextUpdateTime);
                    Render(sumUpdate / nextUpdateTime);
                    ++renderFPS;
                }
               else
               {
                   YieldProcessor();
               }

                if(Timer::GetNowMS() >= nextUpdateFPS)
                {
                    OutputDebugString(L"Render FPS : ");
                    OutputDebugString(std::to_wstring(renderFPS).c_str());
                    OutputDebugString(L"\n");

                    OutputDebugString(L"Update FPS : ");
                    OutputDebugString(std::to_wstring(updateFPS).c_str());
                    OutputDebugString(L"\n");

                    nextUpdateFPS = Timer::GetNowMS() + 1000.0f;

                    updateFPS = 0;
                    renderFPS = 0;
                }

            }
            sumRender += (Timer::GetNowMS() - start);
            sumUpdate += (Timer::GetNowMS() - start);
        }
    }

    programReturn = (int)msg.wParam;
    EXIT_PROGRAM:;
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

    float speed = 10.0f;

    /*
     *  direct input ver
     */

    unsigned char* gKeyboard = gDirectInput->GetKeyboardPress();

    if(!(gDirectInput->GetControlMode() & (uint32)eControlFlags::KEYBOARD_MOVEMENT_MODE))
    {
        int mouseX = 0;
        int mouseY = 0;
        speed = 0.5f;
        gDirectInput->GetMouseDeltaPosition(mouseX, mouseY);
        if(!(mouseX == 0 && mouseY == 0))
        {
            gCamera->RotateAxis(XMConvertToRadians(static_cast<float>(mouseX)) * deltaTime * speed, XMConvertToRadians(static_cast<float>(mouseY)) * deltaTime * speed);
        }

    }
    else
    {
        if (gKeyboard[DIK_W] & 0x80)
        {
            gCamera->RotateAxis( 0.0f, XMConvertToRadians(-(speed * deltaTime)));
        }

        if (gKeyboard[DIK_S] & 0x80)
        {
            gCamera->RotateAxis(0.0f, XMConvertToRadians(speed * deltaTime));
        }
        if (gKeyboard[DIK_A] & 0x80)
        {
            gCamera->RotateAxis(XMConvertToRadians(-(speed * deltaTime)), 0.0f);
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
       // gCamera->AddRadiusSphere( deltaTime);
        gLight->Move(0.001f, -1.0f);
        gLight->SetDirection(gCharacter->GetCenterPoint());

    }

    if (gKeyboard[DIK_E] & 0x80)
    {
        //gCamera->AddRadiusSphere(-deltaTime);
        gLight->Move(-0.001f, 1.0f);
        gLight->SetDirection(gCharacter->GetCenterPoint());
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
        bHightlight = !bHightlight;
        gCharacter->SetHighlight(bHightlight);
    }
    bPressHKey = gKeyboard[DIK_H] & 0x80;

    if(gKeyboard[DIK_Z] & 0x80)
    {
        gCamera->AddHeight(-deltaTime);
    }

    if (gKeyboard[DIK_X] & 0x80)
    {
        gCamera->AddHeight(deltaTime);
    }

    if (gKeyboard[DIK_ESCAPE] & 0x80)
    {
        SendMessage(Renderer::GetInstance()->GetWindowHandle(), WM_DESTROY, 0, 0);
    }


    Renderer::CbViewProj cbViewProj;
    cbViewProj.Matrix = XMMatrixTranspose(gCamera->GetViewProjectionMatrix());
    Renderer::GetInstance()->UpdateCB(Renderer::eCbType::CbViewProj, &cbViewProj);

    
    gLight->Update(gCamera);

    return S_OK;
}

HRESULT Render(float deltaTime)
{
    Renderer::GetInstance()->SetRenderTargetTo(Renderer::eRenderTarget::Default);
    Renderer::GetInstance()->SetViewport(true);
    Renderer::GetInstance()->ClearScreenAndDepth(Renderer::eRenderTarget::Default);

    // 리소스뷰를 어떻게 괜찮은 방법으로 처리할 방법을 검색하기
    gSkybox->Draw();
    gCharacter->Draw();

    gLight->Draw(deltaTime);

    Renderer::GetInstance()->Present();

    return S_OK;
}

void Cleanup()
{
    gDirectInput->Release();
    delete gDirectInput;
    gDirectInput = nullptr;

    delete gPlane;
 //   gImporter->Release();
    delete gSkybox;
    delete gImporter;
    delete gLight;
    delete gCharacter;
    delete gCamera;
 
    // MEMO device가 가장 마지막에 해제되도록.
    Renderer::GetInstance()->Release();
}

void preprocess(float deltaTime)
{
    Renderer::GetInstance()->SetRenderTargetTo(Renderer::eRenderTarget::Shadow);
    Renderer::GetInstance()->SetViewport(false);
    Renderer::GetInstance()->ClearScreenAndDepth(Renderer::eRenderTarget::Shadow);
    
    gCharacter->DrawShadow();
}
