#include <windows.h>

namespace util {

enum class ArchitectureType {
    at_unknown = 0,
    at_x86,
    at_x64,
    at_arm32,
    at_arm64,
};

// Detect the current OS architecture using IsWow64Process2 when available,
// otherwise fall back to GetNativeSystemInfo.
ArchitectureType GetOSArchitecture()
{
    static ArchitectureType arch = []() {
        using FnIsWow64Process2 = BOOL(WINAPI*)(HANDLE, USHORT*, USHORT*);
        auto pIsWow64Process2 = reinterpret_cast<FnIsWow64Process2>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "IsWow64Process2")
        );

        if (pIsWow64Process2) {
            USHORT processMachine = 0;
            USHORT nativeMachine = 0;

            if (pIsWow64Process2(GetCurrentProcess(), &processMachine, &nativeMachine)) {
                switch (nativeMachine) {
                case IMAGE_FILE_MACHINE_I386:
                    return ArchitectureType::at_x86;
                case IMAGE_FILE_MACHINE_AMD64:
                    return ArchitectureType::at_x64;
                case IMAGE_FILE_MACHINE_ARMNT:
                    return ArchitectureType::at_arm32;
                case IMAGE_FILE_MACHINE_ARM64:
                    return ArchitectureType::at_arm64;
                default:
                    return ArchitectureType::at_unknown;
                }
            }

            return ArchitectureType::at_unknown;
        }
        else {
            SYSTEM_INFO si{};
            GetNativeSystemInfo(&si);

            switch (si.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_INTEL:
                return ArchitectureType::at_x86;
            case PROCESSOR_ARCHITECTURE_AMD64:
                return ArchitectureType::at_x64;
            case PROCESSOR_ARCHITECTURE_ARM:
                return ArchitectureType::at_arm32;
            case PROCESSOR_ARCHITECTURE_ARM64:
                return ArchitectureType::at_arm64;
            default:
                return ArchitectureType::at_unknown;
            }
        }
    }();

    return arch;
}

} // namespace util

int main()
{
    const auto arch = util::GetOSArchitecture();
    return arch == util::ArchitectureType::at_x64 ? 0 : 1;
}
