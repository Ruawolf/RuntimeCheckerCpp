#include "Dotnet.h"

#include <nethost.h>
#pragma comment(lib, "nethost.lib")

#include <coreclr_delegates.h>
#include <hostfxr.h>


#include <windows.h>
#include <string>
#include <memory>
#include <iostream>


using std::wstring, std::wcout, std::endl;

void RuntimeChecker::DotnetInfo() {
    RuntimeChecker::load_hostfxr();
}

// nethostライブラリを使い、hostfxrの場所を特定し、エクスポートを取得する。
void RuntimeChecker::load_hostfxr()
{
    size_t buffer_size = 0;
    ::get_hostfxr_path(nullptr, &buffer_size, nullptr);

    wstring buffer(buffer_size - 1, L'\0');
    int retCode = ::get_hostfxr_path(buffer.data(), &buffer_size, nullptr);
    if (retCode != 0) return;

    wcout << buffer << endl;

    
    std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&::FreeLibrary)> dllHandle(::LoadLibraryW(buffer.data()), ::FreeLibrary);
    if (dllHandle == nullptr) return;

    auto getEnvFptr = reinterpret_cast<hostfxr_get_dotnet_environment_info_fn>(::GetProcAddress(dllHandle.get(), "hostfxr_get_dotnet_environment_info"));
    //auto close_fptr = reinterpret_cast<hostfxr_close_fn>(::GetProcAddress(dllHandle.get(), "hostfxr_close"));
    
    int32_t retCode2 = getEnvFptr(nullptr, nullptr, RuntimeChecker::GetEnvCallback, nullptr);
    if (retCode2 != 0) return;

    return;
}

void HOSTFXR_CALLTYPE RuntimeChecker::GetEnvCallback(const struct hostfxr_dotnet_environment_info* info, void* result_context){
    for (size_t i = 0; i < info->sdk_count; i++) {
        auto sdk = info->sdks[i];

        wcout << sdk.version << " : " << sdk.path << endl;
    }

    for (size_t i = 0; i < info->framework_count; i++) {
        auto framework = info->frameworks[i];

        wcout << framework.name << " : " << framework.version << " : " << framework.path << endl;
    }
}
