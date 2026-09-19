#include "pch.h"
#include "ClassFactory.h"
#include "CamStudioGuids.h"
#include <cstdio>

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        printf(">>> CamStudioVirtualCameraW11 DLL LOADED!\n");
        fflush(stdout);
    }

    return TRUE;
}

STDAPI DllGetClassObject(
    REFCLSID clsid,
    REFIID riid,
    LPVOID* ppv)
{
    printf("DllGetClassObject called!\n");
    if (ppv == nullptr)
        return E_POINTER;

    *ppv = nullptr;

    if (!IsEqualCLSID(clsid, CLSID_CamStudioVirtualCamera))
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    ClassFactory* factory = new ClassFactory();
    if (factory == nullptr)
        return E_OUTOFMEMORY;

    HRESULT hr = factory->QueryInterface(riid, ppv);
    factory->Release();
    return hr;
}