#include "Input.h"

Input::Input()
    : mDirectInput(nullptr)
    ,   mKeyboardInput(nullptr)
    , mMouseInput(nullptr)
    , mMouseX(0)
    , mMouseY(0)
    , mScreenWidth(0)
    , mScreenHeight(0)
{
    ZeroMemory(&mMouseState, sizeof(mMouseState));
    ZeroMemory(&mKeyboardState, sizeof(mKeyboardState));
}

Input::~Input()
{

}

HRESULT Input::Initialize(HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight)
{
    HRESULT result = S_OK;

    result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&mDirectInput, nullptr);
    if (FAILED(result))
    {
        return E_FAIL;
    }

    result = mDirectInput->CreateDevice(GUID_SysKeyboard, &mKeyboardInput, nullptr);
    if (FAILED(result))
    {
        return E_FAIL;
    }

    mScreenWidth = screenWidth;
    mScreenHeight = screenHeight;
    //
    // keyboard
    //

    result = mKeyboardInput->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result))
    {
        return E_FAIL;
    }

    result = mKeyboardInput->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(result))
    {
        return E_FAIL;
    }

    result = mKeyboardInput->Acquire();
    if (FAILED(result))
    {
        return E_FAIL;
    }

    ZeroMemory(mKeyboardState, sizeof(mKeyboardState));

    //
    // mouse
    //

    mDirectInput->CreateDevice(GUID_SysMouse, &mMouseInput, nullptr);
    if (FAILED(result))
    {
        return E_FAIL;
    }

    mMouseInput->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result))
    {
        return E_FAIL;
    }

    mMouseInput->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
    if (FAILED(result))
    {
        return E_FAIL;
    }

    result = mMouseInput->Acquire();
    if (FAILED(result))
    {
        return E_FAIL;
    }

    return result;
}

void Input::Release()
{
    mKeyboardInput->Unacquire();
    mMouseInput->Unacquire();

    SAFETY_RELEASE(mKeyboardInput);
    SAFETY_RELEASE(mMouseInput);
}

void Input::UpdateWindowSize(int newWidth, int newHeight)
{
    mScreenWidth = newWidth;
    mScreenHeight = newHeight;
}

HRESULT Input::UpdateInput()
{
    //
    // keyboard
    //

    HRESULT result = mKeyboardInput->GetDeviceState(sizeof(mKeyboardState), &mKeyboardState);
    if (FAILED(result))
    {
        if ((result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED))
        {
            mKeyboardInput->Acquire();

        }
        else
        {
            return E_FAIL;
        }
    }

    //
    // mouse
    //

    result = mMouseInput->GetDeviceState(sizeof(mMouseState), &mMouseState);
    if (FAILED(result))
    {
        if((result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED))
        {
            mMouseInput->Acquire();

        }
        else
        {
            return E_FAIL;
        }
    }
    mMouseX = mScreenWidth / 2;
    mMouseY = mScreenHeight / 2;

    mMouseX += mMouseState.lX;
    mMouseY += mMouseState.lY;

    mOriginalMouseX = mMouseState.lX;
    mOriginalMouseY = mMouseState.lY;

    // clamp
    if(mMouseX < 0)
    {
        mMouseX = 0;
    }
    if (mMouseY < 0)
    {
        mMouseY = 0;
    }

    if (mMouseX > mScreenWidth)
    {
        mMouseX = mScreenWidth;
    }
    if (mMouseY > mScreenHeight)
    {
        mMouseY = mScreenHeight;
    }

    return result;
}

void Input::GetMousePosition(int& mouseX, int& mouseY) const
{
    mouseX = mMouseX;
    mouseY = mMouseY;
}

void Input::GetMouseDeltaPosition(int& deltaX, int& deltaY) const
{
    //deltaX = mMouseState.lX;
    //deltaY = mMouseState.lY;
    deltaX = mOriginalMouseX;
    deltaY = mOriginalMouseY;
}

unsigned char* Input::GetKeyboardPress()
{
    return mKeyboardState;
}
