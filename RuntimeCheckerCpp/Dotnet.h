#pragma once
#include <hostfxr.h>

namespace RuntimeChecker {
    void DotnetInfo();
    void load_hostfxr();
    void HOSTFXR_CALLTYPE GetEnvCallback(const struct hostfxr_dotnet_environment_info* info, void* result_context);
}

