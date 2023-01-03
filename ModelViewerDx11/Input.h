#pragma once
#include "framework.h"


class Input final
{
public:
    Input();
    ~Input();

    HRESULT     Initialize(HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight);
    void        Release();

    void        UpdateWindowSize(int newWidth, int newHeight);

    HRESULT     UpdateInput();

    void                GetMousePosition(int& mouseX, int& mouseY) const;
    void                GetMouseDeltaPosition(int& deltaX, int& deltaY) const;
    unsigned char*      GetKeyboardPress();

private:
    IDirectInput8*           mDirectInput;
    IDirectInputDevice8*     mKeyboardInput;
    IDirectInputDevice8*     mMouseInput;

    unsigned char           mKeyboardState[256];
    DIMOUSESTATE            mMouseState;
    
    int                     mMouseX;
    int                     mMouseY;
    int                     mOriginalMouseX;
    int                     mOriginalMouseY;

    int                     mScreenWidth;
    int                     mScreenHeight;
};