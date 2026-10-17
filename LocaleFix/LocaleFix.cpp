#include "ModContract.h"
#include <Windows.h>
#include <Psapi.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace {
    const ModHost* Host = nullptr;
    const uint8_t* Language = nullptr;
    uint32_t* CodePage = nullptr;
    uint32_t OriginalCodePage = 0;
    bool Resolved = false;

    uintptr_t FindPattern(const uint8_t* Pattern, const char* Mask) {
        MODULEINFO Info{};
        if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(nullptr), &Info, sizeof(Info))) return 0;
        const auto* Base = static_cast<const uint8_t*>(Info.lpBaseOfDll);
        const size_t Length = std::strlen(Mask);
        for (size_t i = 0; i + Length <= Info.SizeOfImage; i++) {
            size_t j = 0;
            while (j < Length && (Mask[j] == '?' || Base[i + j] == Pattern[j])) j++;
            if (j == Length) return reinterpret_cast<uintptr_t>(Base + i);
        }
        return 0;
    }

    void Resolve() {
        if (Resolved) return;
        Resolved = true;
        const uint8_t LanguagePattern[] = {0xA1, 0, 0, 0, 0, 0x80, 0x38, 0x08, 0x75, 0, 0xB2, 0x01, 0xB0, 0x1F};
        if (const uintptr_t Match = FindPattern(LanguagePattern, "x????xxxx?xxxx")) {
            Language = **reinterpret_cast<const uint8_t* const* const*>(Match + 1);
        }
        const uint8_t CodePagePattern[] = {0x66, 0x83, 0xF8, 0x04, 0x76, 0, 0xC7, 0x05, 0, 0, 0, 0, 0x03, 0x00, 0x00, 0x00};
        if (const uintptr_t Match = FindPattern(CodePagePattern, "xxxxx?xx????xxxx")) {
            CodePage = *reinterpret_cast<uint32_t**>(Match + 8);
            OriginalCodePage = *CodePage;
        }
    }

    LCID GameLocale() {
        if (!Language) return 0;
        switch (*Language) {
            case 0: return MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK), SORT_DEFAULT);
            case 1: return MAKELCID(MAKELANGID(LANG_GERMAN, SUBLANG_DEFAULT), SORT_DEFAULT);
            case 2: return MAKELCID(MAKELANGID(LANG_FRENCH, SUBLANG_DEFAULT), SORT_DEFAULT);
            case 3: return MAKELCID(MAKELANGID(LANG_ITALIAN, SUBLANG_DEFAULT), SORT_DEFAULT);
            case 4: return MAKELCID(MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT), SORT_DEFAULT);
            case 5: return MAKELCID(MAKELANGID(LANG_SPANISH, SUBLANG_DEFAULT), SORT_DEFAULT);
            case 6: return MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT), SORT_DEFAULT);
            case 7: return MAKELCID(MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT), SORT_DEFAULT);
            case 8: return MAKELCID(MAKELANGID(LANG_TURKISH, SUBLANG_DEFAULT), SORT_DEFAULT);
            case 9: return MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_DEFAULT);
            default: return 0;
        }
    }

    uint32_t GameCodePage() {
        uint32_t Result = 0;
        const LCID Locale = GameLocale();
        if (!Locale || !GetLocaleInfoA(Locale, LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
                                       reinterpret_cast<LPSTR>(&Result), sizeof(Result) / sizeof(char))) return 0;
        return Result;
    }

    uint32_t ConfiguredCodePage() {
        char Path[MAX_PATH]{};
        GetModuleFileNameA(nullptr, Path, MAX_PATH);
        std::string Ini = Path;
        Ini = Ini.substr(0, Ini.find_last_of("\\/") + 1) + "mods\\LocaleFix.ini";
        const UINT Value = GetPrivateProfileIntA("LocaleFix", "CodePage", 0, Ini.c_str());
        return Value && IsValidCodePage(Value) ? Value : 0;
    }

    void Apply() {
        Resolve();
        static const uint32_t Configured = ConfiguredCodePage();
        const uint32_t Wanted = Configured ? Configured : GameCodePage();
        if (!CodePage || !Wanted) {
            if (Host) Host->ReportStatus(ModHealthLevel::Broken, !CodePage ? "Code page variable not found in this game build."
                                                                          : "Client language not found in this game build.");
            return;
        }
        if (*CodePage != Wanted) *CodePage = Wanted;
        if (!Host) return;
        char Message[96];
        std::snprintf(Message, sizeof(Message), "Text code page pinned to %u (was %u). Resize once to redraw.", Wanted, OriginalCodePage);
        Host->ReportStatus(ModHealthLevel::Ok, Message);
    }
}

extern "C" {
    __declspec(dllexport) const ModClassRequirement* ModGetRequirements(size_t* OutCount) {
        *OutCount = 0;
        return nullptr;
    }

    __declspec(dllexport) void ModStartup(ImGuiContext*, ImGuiMemAllocFunc, ImGuiMemFreeFunc, void*, const ModHost* ModHostPtr) {
        Host = ModHostPtr;
        Apply();
    }

    __declspec(dllexport) void ModShutdown() {
        if (CodePage && OriginalCodePage) *CodePage = OriginalCodePage;
        Host = nullptr;
    }

    __declspec(dllexport) void ModEarlyTick(TLBSWidget*, TickContext) {
        Apply();
    }

    __declspec(dllexport) void ModTick(TLBSWidget*, TickContext) {}

    __declspec(dllexport) void ModToggleMainWindow() {}
}
