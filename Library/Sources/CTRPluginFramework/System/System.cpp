#include <Headers.hpp>
#include <string>
#include <unordered_map>

namespace CTRPluginFramework {
    void (*System::OnAbort)(void) = nullptr;

    // Plugin-injected UI text table (i18n bridge for framework-owned labels/prompts).
    static std::unordered_map<std::string, std::string> g_fwText;
    void SetFrameworkText(const std::string &key, const std::string &value) { g_fwText[key] = value; }
    std::string FwText(const std::string &key, const std::string &fallback) {
        auto it = g_fwText.find(key);
        return (it != g_fwText.end() && !it->second.empty()) ? it->second : fallback;
    }

    // Active UI language code, pushed by the plugin (empty/"en" => English).
    static std::string g_fwLang;
    void SetFrameworkLang(const std::string &code) { g_fwLang = code; }
    std::string FwLang(void) { return g_fwLang; }

    bool System::IsNew3DS(void) {
        return (SystemImpl::IsNew3DS);
    }

    bool System::IsCitra(void) {
        return (SystemImpl::IsCitra);
    }

    bool System::IsSleeping(void) {
        return (SystemImpl::IsSleeping());   // true once the lid is closing (WANTSTOSLEEP) or asleep (SLEEPING)
    }

    LanguageId System::GetSystemLanguage(void) {
        return (static_cast<LanguageId>(SystemImpl::Language));
    }

    bool System::IsConnectedToInternet(void) {
        u32 out = 0;
        u8 *wifiLevel = (u8*)0x1FF81066;

        if (R_FAILED(ACU_GetWifiStatus(&out)))
            out = 0;

        return (out != 0 && *wifiLevel != 0);
    }

    /// Retrieves the major version from a packed system version.
    #define GET_VERSION_MAJOR(version) ((version) >>24)

    /// Retrieves the minor version from a packed system version.
    #define GET_VERSION_MINOR(version) (((version)>>16)&0xFF)

    /// Retrieves the revision version from a packed system version.
    #define GET_VERSION_REVISION(version) (((version)>> 8)&0xFF)

    bool System::CfwIsLuma3DS(const u8 major, const u8 minor, const u8 revision) {
        if (SystemImpl::CFWVersion == 0 || !major)
            return (false);

        u8 _major = GET_VERSION_MAJOR(SystemImpl::CFWVersion);
        u8 _minor = GET_VERSION_MINOR(SystemImpl::CFWVersion);
        u8 _revision = GET_VERSION_REVISION(SystemImpl::CFWVersion);

        if (_major < major)
            return (false);

        if (_minor < minor)
            return (false);

        return (_revision >= revision);
    }

    void System::Reboot(void) {
        svcKernelSetState(7);
        for (;;);
    }
}