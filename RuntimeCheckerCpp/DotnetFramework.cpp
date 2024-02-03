#include "DotnetFramework.h"

// mscoree.dll動的ロード
#define DynamicLoad

#include <iostream>
#include <memory>
#include <vector>

#include <Windows.h>
#include <atlbase.h>

#ifdef _DEBUG
// デバッグ用
#pragma comment(lib, "atls.lib")
#endif

#include <metahost.h>
#ifndef DynamicLoad
#pragma comment(lib, "mscoree.lib")
#endif

using std::wstring, std::wcout, std::endl;

void RuntimeChecker::DotnetFrameworkInfo()
{
    wcout.imbue(std::locale("Japanese"));

#ifdef DynamicLoad
    std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&::FreeLibrary)> dllHandle(::LoadLibraryW(L"mscoree.dll"), ::FreeLibrary);
    if (dllHandle == nullptr) {
        wcout << L"LoadLibraryWエラー" << endl;
        std::exit(EXIT_FAILURE);
    }

    auto procAddress = ::GetProcAddress(dllHandle.get(), "CLRCreateInstance");
    if (procAddress == nullptr) {
        wcout << L"GetProcAddressエラー" << endl;
        std::exit(EXIT_FAILURE);
    }

    auto CLRCreateInstanceFunc = reinterpret_cast<HRESULT(__stdcall*)(REFCLSID clsid, REFIID riid, LPVOID * ppInterface)>(procAddress);
#endif

    HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        wcout << L"COM初期化エラー" << endl;
        std::exit(EXIT_FAILURE);
    }

    CComPtr<ICLRMetaHost> pMetaHost;

#ifdef DynamicLoad
    hr = CLRCreateInstanceFunc(CLSID_CLRMetaHost, IID_ICLRMetaHost, reinterpret_cast<void**>(&pMetaHost));
#else
    hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, reinterpret_cast<void**>(&pMetaHost));
#endif

    if (SUCCEEDED(hr)) {
        CComPtr<IEnumUnknown> pRuntimeIterator;
        pMetaHost->EnumerateInstalledRuntimes(&pRuntimeIterator);

        IUnknown* pIUnknown = nullptr;

        while (pRuntimeIterator->Next(1, &pIUnknown, nullptr) == S_OK) {
            CComPtr<ICLRRuntimeInfo> pRuntimeInfo;
            HRESULT hr = pIUnknown->QueryInterface<ICLRRuntimeInfo>(&pRuntimeInfo);

            if (SUCCEEDED(hr)) {
                if (pRuntimeInfo != NULL) {
                    DWORD bufferSize = 0;
                    pRuntimeInfo->GetVersionString(nullptr, &bufferSize);
                    wstring text(bufferSize - 1, L'\0');
                    pRuntimeInfo->GetVersionString(text.data(), &bufferSize);

                    wcout << text << endl;
                }
            }
            pIUnknown->Release();
        }
    }
}
