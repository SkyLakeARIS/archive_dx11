// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

//#pragma comment(lib, "libfbxsdk.lib")
//#pragma comment(lib, "libfbxsdk-md.lib")
//#pragma comment(lib, "libfbxsdk-mt.lib")

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

// d3d11 header files
//#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectxTex.h>
#include <DirectXColors.h>
#include <dxgidebug.h>
// Windows 헤더 파일
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// macro

#define SAFETY_RELEASE(obj)     \
 if((obj) != nullptr)           \
{                               \
     obj->Release();            \
     obj = nullptr;             \
}                               \

#define ASSERT(expr, msg)                                               \
 if(!(expr))                                                            \
{                                                                       \
     fprintf(stderr, "%s (%d), msg: %s\n", __FILE__, __LINE__, msg);    \
     __debugbreak();                                                    \
}                                                                       \

#ifdef _DEBUG
#define SET_PRIVATE_DATA(obj, objectNameStr)         \
    if(obj != nullptr)                               \
    {                                                \
        obj->SetPrivateData(                         \
            WKPDID_D3DDebugObjectName,               \
            sizeof(objectNameStr)-1,                 \
            objectNameStr);                          \
    }                                                \

#else
#define SET_PRIVATE_DATA(obj, objectNameStr)
#endif
