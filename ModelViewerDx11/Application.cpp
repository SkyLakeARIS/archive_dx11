#include "Application.h"
#include <algorithm>
#include <string>
#include "Window.h"
#include "Renderer/Renderer.h"
#include "Renderer/Importer/ModelImporter.h"
#include "Renderer/Primitive/MeshGenerator.h"
#include "Renderer/Resources/BufferManager.h"
#include "Renderer/Resources/Model.h"
#include "Renderer/Resources/ResourceManager.h"
#include "Renderer/Resources/TextureManager.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Scene/Billboard.h"
#include "Scene/Camera.h"
#include "Scene/Floor.h"
#include "Scene/Light.h"
#include "Scene/Sky.h"
#include "UI/DebugPanel.h"
#include "Util/Macro.h"


bool RenderPacketCompareDecr(const renderer::RenderPacket& lhs, const renderer::RenderPacket& rhs)
{
    return lhs.SortKey > rhs.SortKey;
}

Application::Application()
    : mWindowWidth(1280)
    , mWindowHeight(720)
    , mAppFrameRate(120)
    , mWindow(nullptr)
    , mCommandCache()
    , mRenderer(nullptr)
    , mImporter(nullptr)
    , mCharacter(nullptr)
    , mCamera(nullptr)
    , mSkybox(nullptr)
    , mLight(nullptr)
    , mLightIcon(nullptr)
    , mFloor(nullptr)
    , mBufferManager(nullptr)
    , mTextureManager(nullptr)
    , mResourceManager(nullptr)
    , mShaderManager(nullptr)
    , mDirectInput(nullptr)
    , mShadowDebugPanel(nullptr)
{
    mRenderer = new renderer::Renderer();
    mImporter = new renderer::ModelImporter();
    mCommandList.reserve(64);
}

Application::~Application()
{
    std::vector<renderer::RenderPacket>().swap(mCommandList);
    delete mShadowDebugPanel;
    mDirectInput->Release();
    delete mDirectInput;
    mDirectInput = nullptr;
    delete mFloor;
    delete mSkybox;
    delete mImporter;
    delete mLight;
    delete mLightIcon;
    delete mCharacter;
    delete mCamera;

    delete mWindow;
    delete mBufferManager;
    delete mTextureManager;
    delete mResourceManager;
    delete mShaderManager;
    delete mRenderer;
#ifdef _DEBUG
    renderer::Renderer::CheckLiveObjects();
#endif
}

bool Application::InitializeWithWindows(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int32_t nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    mWindow = new Window(hInstance);

    mWindow->RegisterWindowClass();

    const HWND handleWindow = mWindow->MakeWindow(mWindowWidth, mWindowHeight);
    if(!handleWindow)
    {
        return false;
    }
 
    mWindow->DisplayWindow(nCmdShow);
    mWindow->RefreshWindow();

    
    if (FAILED(mRenderer->initialize(handleWindow, mWindowWidth, mWindowHeight, mAppFrameRate)))
    {
        ASSERT(false, "Fail to initialize shaders");
        return false;
    }

    mDirectInput = new core::DirectInput(hInstance, handleWindow, mWindowWidth,
                                         mWindowHeight);
    if (FAILED(mDirectInput->Initialize()))
    {
        ASSERT(false, "모델데이터 초기화 실패 SetupGeometry");
        return false;
    }
    mImporter->Initialize();

    if(!initializeManagers())
    {
        ASSERT(false, "fail to initialize managers");
    }

    initializeScene();

    return true;
}

void Application::Run()
{
    const float RenderIntervalTime = 1000.0f / static_cast<float>(mAppFrameRate);

    core::Timer::Tick();
    // MEMO: 첫 프레임이 안정적으로 돌도록 함. 프로그램 내에서 FrameTime이 일관되도록
    double lastFrameTime = core::Timer::GetNowMS();
    double lastFPSTime = core::Timer::GetNowMS();
    int16_t frameCount = 0;
    bool bReinitDevice = false;
    while (true)
    {
        if (bReinitDevice)
        {
            mRenderer->Cleanup();
            mRenderer->initialize(mWindow->GetHandle(), mWindowWidth, mWindowHeight, mAppFrameRate);
            bReinitDevice = false;
        }

        if(mWindow->ProcessMessages())
        {
            break;
        }

        core::Timer::Tick();

        const double startTime = core::Timer::GetNowMS();
        double deltaTime = startTime - lastFrameTime;
        // MEMO: 디버거 대응. deltaTime이 너무 크면 시간을 재조정한다.
        if(deltaTime > 100.0)
        {
            deltaTime = RenderIntervalTime;
        }

        if (RenderIntervalTime > deltaTime)
        {
            YieldProcessor();
            continue;
        }

        lastFrameTime = startTime;

        mDirectInput->UpdateInput();

        updateScene(deltaTime);

        preprocess();
        renderScene();
        renderUI();
        mRenderer->Present();

        mBufferManager->MarkInvalidateDynamicBuf();

        ++frameCount;
        if (startTime - lastFPSTime >= 1000.0)
        {
            OutputDebugString(L"Render FPS : ");
            OutputDebugString(std::to_wstring(frameCount).c_str());
            OutputDebugString(L"\n");
            lastFPSTime += 1000.0;
            frameCount = 0;
        }

        if (mRenderer->CheckDeviceLost(bReinitDevice))
        {
            break;
        }
    }
}


bool Application::initializeScene()
{
    core::Timer::Initialize();

    mCamera = new scene::Camera(
        XMVectorSet(0.0f, 10.0f, -15.0f, 0.0f)
        , XMVectorSet(0.0f, 10.0f, 0.0f, 0.0f)
        , XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        mWindowWidth,
        mWindowHeight);




    const int8_t* const modelFilePath = reinterpret_cast<int8_t*>("/AssetData/models/unagi.fbx");
    // TODO: 나중에 Object List를 만들어서 관리하도록 변경(성공하면 drawable 리스트에 추가)
    mCharacter = new renderer::Model(mCamera, mBufferManager);

    mResourceManager->LoadModel(modelFilePath, mCharacter);

    mSkybox = new scene::Sky(*mCamera);
    mSkybox->Initialize(10, 10, mTextureManager);

    mRenderer->BindPrimitiveTopologyTo(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCamera->ChangeFocus(mCharacter->GetCenterPoint(), *mRenderer);
    // MEMO Light 위치값 막 바꾸면 안됨. 그림자 제대로 안그려질 수 있음. 나중에 개선해야 할 항목 중 하나(cascade)
  //  gLight = new Light(XMFLOAT3(0.0f, 50.0f, 70.0f), gCharacter->GetCenterPoint(), XMFLOAT3(1.0f, 1.0f, 1.0f), gCamera, 0.1f, 300.0f);

    mLight = new scene::Light(XMFLOAT3(0.0f, 20.0f, 50.0f), mCharacter->GetCenterPoint(), XMFLOAT3(1.0f, 1.0f, 1.0f), mCamera, 0.1f, 500.0f, *mRenderer);
    mLight->SetupCascade(*mRenderer);

    mFloor = new scene::Floor(XMFLOAT2(0.0f, 0.0f), 2, 10, 10);

    mShadowDebugPanel = new ui::DebugPanel(0, 0, 200, 200);
    mShadowDebugPanel->SetDebugType(renderer::eRenderTarget::Shadow);

    mLightIcon = new scene::Billboard();
    mLightIcon->Initialize(*mRenderer);

    const int8_t* const filePath = reinterpret_cast<const int8_t*>("./AssetData/textures/lightIcon.png");
    HashID lightIconTexID = 0;
    mTextureManager->AddTexture(filePath, lightIconTexID);
    ASSERT(lightIconTexID, "icon texture fail to add");

    mLightIcon->SetTexture(lightIconTexID);

    // MEMO: 유효하지 않은 상태로 세팅
    mCommandCache = {};
    mCommandCache.VertexFormat = renderer::eVertexFormat::FormatCount;
    mCommandCache.Stride = 0;
    mCommandCache.RenderState.ShaderType = renderer::eShader::ShaderCount;
    mCommandCache.RenderState.TopologyType = renderer::ePrimitiveTopology::TopologyCount;
    mCommandCache.RenderState.SamplerType = renderer::eSamplerType::SamplerCount;
    mCommandCache.RenderState.RasterType = renderer::eRasterType::RasterCount;
    mCommandCache.RenderState.BlendHash = 0;
    return true;
}

bool Application::initializeManagers()
{
    ID3D11Device* device = mRenderer->GetDevice();
    ID3D11DeviceContext* deviceContext = mRenderer->GetDeviceContext();

    mShaderManager = new renderer::ShaderManager(*device, *mRenderer);

    mBufferManager = new renderer::BufferManager(device, deviceContext, renderer::BufferManager::eIndexListFormat::UInt32);
    if (!mBufferManager->Initialize(renderer::BufferManager::sVertexBufferDefaultSize, renderer::BufferManager::sIndexBufferDefaultSize, renderer::BufferManager::sVertexBufferDefaultSize, renderer::BufferManager::sIndexBufferDefaultSize))
    {
        ASSERT(false, "buffer manager init failed.")
        return false;
    }

    mTextureManager = new renderer::TextureManager(device);
    mResourceManager = new renderer::ResourceManager(device, mTextureManager, mImporter, mBufferManager);

    mRenderer->SetManagers(mBufferManager, mTextureManager);
    renderer::MeshGenerator::Initialize(mBufferManager);
    return true;
}

void Application::updateScene(double deltaTime)
{
    float speed = 10.0f;

    /*
     *  direct input ver
     */

    unsigned char* gKeyboard = mDirectInput->GetKeyboardPress();

    if (!(mDirectInput->GetControlMode() & (uint32)core::eControlFlags::KEYBOARD_MOVEMENT_MODE))
    {
        int mouseX = 0;
        int mouseY = 0;
        speed = 0.5f;
        mDirectInput->GetMouseDeltaPosition(mouseX, mouseY);
        if (!(mouseX == 0 && mouseY == 0))
        {
            mCamera->RotateAxis(XMConvertToRadians(static_cast<float>(mouseX)) * deltaTime * speed, XMConvertToRadians(static_cast<float>(mouseY)) * deltaTime * speed, *mRenderer);
        }
    }
    else
    {
        if (gKeyboard[DIK_W] & 0x80)
        {
            mCamera->RotateAxis(0.0f, XMConvertToRadians(-(speed * deltaTime)), *mRenderer);
        }

        if (gKeyboard[DIK_S] & 0x80)
        {
            mCamera->RotateAxis(0.0f, XMConvertToRadians(speed * deltaTime), *mRenderer);
        }
        if (gKeyboard[DIK_A] & 0x80)
        {
            mCamera->RotateAxis(XMConvertToRadians(-(speed * deltaTime)), 0.0f, *mRenderer);
        }

        if (gKeyboard[DIK_D] & 0x80)
        {
            mCamera->RotateAxis(XMConvertToRadians(speed * deltaTime), 0.0f, *mRenderer);
        }
    }

    // 마우스 휠 처리 이전에 임시용.
    // 카메라와 물체간의 거리 조절(구체 크기 확대/축소)
    if (gKeyboard[DIK_Q] & 0x80)
    {
        mCamera->AddRadiusSphere(deltaTime, *mRenderer);
    }

    if (gKeyboard[DIK_E] & 0x80)
    {
        mCamera->AddRadiusSphere(-deltaTime, *mRenderer);
    }

    // 키보드<-> 마우스 조작 전환
    static bool bPressKey = false;
    if (!(gKeyboard[DIK_C] & 0x80) && bPressKey)
    {
        mDirectInput->SetControlMode((uint32)core::eControlFlags::KEYBOARD_MOVEMENT_MODE);
    }
    bPressKey = gKeyboard[DIK_C] & 0x80;

    static bool bPressHKey = false;
    static bool bHightlight = false;
    if (!(gKeyboard[DIK_H] & 0x80) && bPressHKey)
    {
        bHightlight = !bHightlight;
        mCharacter->SetHighlight(bHightlight);
    }
    bPressHKey = gKeyboard[DIK_H] & 0x80;

    if (gKeyboard[DIK_Z] & 0x80)
    {
        mCamera->AddHeight(-deltaTime, *mRenderer);
    }

    if (gKeyboard[DIK_X] & 0x80)
    {
        mCamera->AddHeight(deltaTime, *mRenderer);
    }

    if (gKeyboard[DIK_ESCAPE] & 0x80)
    {
        SendMessage(mWindow->GetHandle(), WM_DESTROY, 0, 0);
    }


    renderer::CbViewProj cbViewProj;
    cbViewProj.Matrix = XMMatrixTranspose(mCamera->GetViewProjectionMatrix());
    mRenderer->UpdateCB(renderer::eCbType::CbViewProj, &cbViewProj);


    mLight->SetupCascade(*mRenderer);
    // TODO: improve - 이후에 창 크기 말고 viewport 사이즈로 바꾸는 것으로 검토(급하진 않음)
   const XMMATRIX uiProjMat = XMMatrixOrthographicOffCenterLH(0.0, mWindowWidth, mWindowHeight, 0.0, 0.1f, 100.0f);
    renderer::CbScreenSpaceMatrix cbScreenSpaceMatrix = {};
    cbScreenSpaceMatrix.Matrix = XMMatrixTranspose(uiProjMat);
    mRenderer->UpdateCB(renderer::eCbType::CbOrthoMatrix, &cbScreenSpaceMatrix);

    mLightIcon->SetPosition(mLight->GetPosition());
    mLightIcon->UpdateScaleMatrix(*mCamera);

    mSkybox->Update(*mRenderer);
    mCharacter->Update(*mRenderer);


    mCommandList.clear();

    mSkybox->Draw(mCommandList);

    mFloor->Draw(mCommandList);

    mCharacter->DrawShadow(mCommandList);
    mCharacter->Draw(mCommandList);

    mLight->DrawDebug(mCommandList);
    mLightIcon->Draw(mCommandList);

    mShadowDebugPanel->Draw(mCommandList);

    for(auto& command : mCommandList)
    {
        renderer::Renderer::MakeSortKey(command);
    }

    std::sort(mCommandList.begin(), mCommandList.end(), RenderPacketCompareDecr);
}

void Application::preprocess()
{
}

void Application::renderScene()
{
    // FIXME: 카메라 거리별로 정렬하지 않아서 icon이 먼저 그려지면서 아무것도 없는 배경과 블렌딩이 됨. (투명/불투명을 먼저 구분해야 함)
    // TODO: 이부분도 렌더링 전에 깔끔하게 세팅 될 수 있도록 해보자.
    mRenderer->ClearScreenAndDepth(renderer::eRenderTarget::Shadow);
    mRenderer->ClearScreenAndDepth(renderer::eRenderTarget::Default);

    for (auto& command : mCommandList)
    {
        if (mCommandCache.RenderTargetType != command.RenderTargetType)
        {
            mRenderer->BindRenderTargetTo(command.RenderTargetType);
            mCommandCache.RenderTargetType = command.RenderTargetType;

            // MEMO: 현재 렌더패킷에 정보가 있지 않아서 이렇게 처리.
            // TODO: 생각해보면 이게 Viewport인데 렌더 패킷에 고려하지 못한 것 같다. 현재 큰 문제는 없으나, 해당 부분은 천천히 작업 필요
            mRenderer->SetViewport(command.RenderTargetType == renderer::eRenderTarget::Default);
        }

        // MEMO: 이건 true 때만 지워야 한다. 현재 바인드된 렌더타겟 초기화
        if (command.RenderState.bClearDepthStencilBuffer)
        {
            mRenderer->ClearScreenAndDepth(mCommandCache.RenderTargetType);
        }

        bool bNeedBindBuffer = (mCommandCache.bUseDynamicBuffer != command.bUseDynamicBuffer) || (mCommandCache.Stride != command.Stride);

        // MEMO: Buffer는 Stride 별로 Chunk가 나뉘어져 있기 때문에 Stride가 달라도 Bind를 다시 해줘야 함.
        if (mCommandCache.VertexFormat != command.VertexFormat)
        {
            mRenderer->BindInputLayoutTo(command.VertexFormat);
            bNeedBindBuffer = true;
            mCommandCache.VertexFormat = command.VertexFormat;
        }

        if(bNeedBindBuffer)
        {
            ASSERT(command.Stride > 0, "유효하지 않은 버퍼이거나 올바르지 않은 command. stride(%d)", command.Stride);
            if (command.bUseDynamicBuffer)
            {
                mRenderer->BindVertexBufferDynamic(command.Stride);
                mRenderer->BindIndexBufferDynamic();
            }
            else
            {
                mRenderer->BindVertexBuffer(command.Stride);
                mRenderer->BindIndexBuffer();
            }
            mCommandCache.bUseDynamicBuffer = command.bUseDynamicBuffer;
            mCommandCache.Stride = command.Stride;
        }

        if (mCommandCache.RenderState.TopologyType != command.RenderState.TopologyType)
        {
            mRenderer->BindPrimitiveTopologyByType(command.RenderState.TopologyType);
            mCommandCache.RenderState.TopologyType = command.RenderState.TopologyType;
        }

        if (mCommandCache.RenderState.ShaderType != command.RenderState.ShaderType)
        {
            mRenderer->BindShaderTo(command.RenderState.ShaderType);
            mCommandCache.RenderState.ShaderType = command.RenderState.ShaderType;

            // MEMO: Renderer 예약 Slot 바인딩
            mRenderer->BindCbToVsByType(0, 1, renderer::eCbType::CbWorld);
            mRenderer->BindCbToVsByType(1, 1, renderer::eCbType::CbViewProj);
            mRenderer->BindCbToVsByType(2, 1, renderer::eCbType::CbLightViewProjMatrix);
            mRenderer->BindCbToVsByType(3, 1, renderer::eCbType::CbLightProperty);
            mRenderer->BindCbToVsByType(4, 1, renderer::eCbType::CbCameraPosition);
            mRenderer->BindCbToVsByType(5, 1, renderer::eCbType::CbOrthoMatrix);

            // MEMO: Material 바인딩
            if (command.RenderState.CbBindingDesc.BindSlot >= 0)
            {
                mRenderer->BindCbToPs(command.RenderState.CbBindingDesc.BindSlot, 1, command.RenderState.CbBindingDesc.Type);
            }
        }

        for(uint8_t texture = static_cast<uint8_t>(renderer::eTextureType::Diffuse); texture < static_cast<uint8_t>(renderer::eTextureType::TextureTypeCount); ++texture)
        {
            if(command.RenderState.TexBindingSlots[texture] < 0)
            {
                continue;
            }

            if (static_cast<renderer::eTextureType>(texture) == renderer::eTextureType::Shadow && command.RenderState.bUseShadowMap)
            {
                mRenderer->BindShadowTextureToPs(command.RenderState.TexBindingSlots[static_cast<uint8_t>(renderer::eTextureType::Shadow)]);
                mCommandCache.RenderState.bUseShadowMap = command.RenderState.bUseShadowMap;
            }
            else if (command.Material.TextureHashes[texture])
            {
                mRenderer->BindTextureToPs(command.RenderState.TexBindingSlots[texture], command.Material.TextureHashes[texture]);
            }
        }
        

        // TODO: 렌더큐 끝나면 이것도 좀 더 명확하게 개선해 봐야 할 항목.
        // MEMO: 이름은 이상하지만 우선은 SkyBox 전용.
        if(mCommandCache.RenderState.bUseDepthStencil != command.RenderState.bUseDepthStencil)
        {
            mRenderer->BindDepthStencilState(command.RenderState.bUseDepthStencil);
            mCommandCache.RenderState.bUseDepthStencil = command.RenderState.bUseDepthStencil;
        }

        if (mCommandCache.RenderState.SamplerType != command.RenderState.SamplerType || mCommandCache.RenderState.SamplerBindingSlot != command.RenderState.SamplerBindingSlot)
        {
            // TODO: -1 일 때는 unbind나 기본값으로 하는 게 좋아보이는데, 이전 값을 그대로 쓰는게 더 나을지 조사가 필요함
            if(command.RenderState.SamplerBindingSlot >= 0)
            {
                mRenderer->BindSamplerToPsByType(command.RenderState.SamplerBindingSlot, command.RenderState.SamplerType);
                mCommandCache.RenderState.SamplerType = command.RenderState.SamplerType;
                mCommandCache.RenderState.SamplerBindingSlot = command.RenderState.SamplerBindingSlot;
            }
        }

        if (mCommandCache.RenderState.RasterType != command.RenderState.RasterType)
        {
            mRenderer->BindRasterStateByType(command.RenderState.RasterType);
            mCommandCache.RenderState.RasterType = command.RenderState.RasterType;
        }

        if (mCommandCache.RenderState.BlendHash != command.RenderState.BlendHash)
        {
            // MEMO: blendFactor는 아직 사용하지 않음.
            mRenderer->BindBlendStateByHash(command.RenderState.BlendHash, nullptr, 0xffffffff);
            mCommandCache.RenderState.BlendHash = command.RenderState.BlendHash;
        }

        // MEMO: Material 식별자가 없는 상태이므로 우선은 매번 업로드
        if (command.RenderState.CbBindingDesc.BindSlot >= 0)
        {
            mShaderManager->UpdateMaterial(command.RenderState.CbBindingDesc.Type, command.Material);
        }

        const renderer::CbWorld cbMatWorld = { command.MatWorld };
        mRenderer->UpdateCB(renderer::eCbType::CbWorld, &cbMatWorld);

        if(command.IndexRange.Count)
        {
            mRenderer->DrawIndexed(command.IndexRange.Count, command.IndexRange.StartIndex, command.VertexRange.StartIndex);
        }
        else
        {
            mRenderer->Draw(command.VertexRange.Count, command.VertexRange.StartIndex);
        }

        // TODO: 자주 호출될 것 같은데, 확인해 보고 최대한 Bind-UnBind를 덜할 수 있는 방법을 다시 고민해보자.
        // MEMO: Shadow RenderTarget으로 써야 하므로 다시 Texture Slot에서 제거.
        if (command.RenderState.bUseShadowMap)
        {
            mRenderer->UnbindTexturePs(command.RenderState.TexBindingSlots[static_cast<uint8_t>(renderer::eTextureType::Shadow)]);
        }
    }
}

void Application::renderUI()
{
}
