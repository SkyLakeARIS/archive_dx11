#pragma once
#include "framework.h"
#include "Renderer/Resources/RenderPacket.h"


namespace ui
{
    class DebugPanel;
}

class Window;

namespace scene
{
    class Billboard;
    class Floor;
    class Light;
    class Sky;
    class Camera;
}

namespace renderer
{
    class Model;
    class ModelImporter;
    class TextureManager;
    class BufferManager;
    class ResourceManager;
    class Renderer;
    class ShaderManager;
}

class Application
{
public:

    Application();
    ~Application();

    bool InitializeWithWindows(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int32_t nCmdShow);

    void Run();

private:
    bool initializeScene();
    bool initializeManagers();

    // TODO: input update와 scene update 로직 분리하기
    void updateScene(double deltaTime);

    void renderScene();

private:

    int16_t mWindowWidth;
    int16_t mWindowHeight;
    int16_t mAppFrameRate;

    Window* mWindow;

    std::vector<renderer::RenderPacket> mCommandList;
    renderer::RenderPacketCache mCommandCache;
    renderer::Renderer* mRenderer;
    renderer::ModelImporter* mImporter;
    renderer::Model* mCharacter;
    scene::Camera* mCamera;
    scene::Sky* mSkybox;
    scene::Light* mLight;
    scene::Billboard* mLightIcon;
    scene::Floor* mFloor;
    renderer::BufferManager* mBufferManager;
    renderer::TextureManager* mTextureManager;
    renderer::ResourceManager* mResourceManager;
    renderer::ShaderManager* mShaderManager;
    core::DirectInput* mDirectInput;
    ui::DebugPanel* mShadowDebugPanel;
};
