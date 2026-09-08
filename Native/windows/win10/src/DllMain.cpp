#include "Windows10VirtualCamera.h"

#include <streams.h>
#include <windows.h>

extern "C"
BOOL WINAPI DllEntryPoint(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
);

const AMOVIESETUP_PIN sudCamStudioPins[] =
{
    {
        L"Video",               // strName
        FALSE,                  // bRendered
        TRUE,                   // bOutput
        FALSE,                  // bZero
        FALSE,                  // bMany
        &CLSID_NULL,            // clsConnectsToFilter
        NULL,                   // strConnectsToPin
        1,                      // nMediaTypes
        nullptr                 // lpMediaType
    }
};

const AMOVIESETUP_MEDIATYPE sudCamStudioMediaTypes[] =
{
    {
        &MEDIATYPE_Video,
        &MEDIASUBTYPE_RGB24
    }
};

const AMOVIESETUP_PIN sudCamStudioPin =
{
    L"Video",
    FALSE,
    TRUE,
    FALSE,
    FALSE,
    &CLSID_NULL,
    NULL,
    1,
    sudCamStudioMediaTypes
};

const AMOVIESETUP_FILTER sudCamStudioFilter =
{
    &CLSID_CamStudioVirtualCamera,
    L"Cam Studio Virtual Camera",
    MERIT_NORMAL,
    1,
    &sudCamStudioPin
};

CFactoryTemplate g_Templates[] =
{
    {
        L"Cam Studio Virtual Camera",
        &CLSID_CamStudioVirtualCamera,
        Windows10VirtualCamera::CreateInstance,
        NULL,
        &sudCamStudioFilter
    }
};


int g_cTemplates =
sizeof(g_Templates) /
sizeof(g_Templates[0]);

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved
)
{
    return DllEntryPoint(
        hModule,
        ul_reason_for_call,
        lpReserved
    );
}

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}


STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}