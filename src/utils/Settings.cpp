#include <Geode/Geode.hpp>

#include "Settings.hpp"

using namespace geode::prelude;

float Settings::_luaMaxExecutionTime;
int Settings::_luaInterruptResolution;
float Settings::_luaUninterruptibleGraceTime;
float Settings::_consoleFontSize;
float Settings::_consoleLineHeightMultiplier;

template<auto& S, class T = std::remove_reference_t<decltype(S)>>
static constexpr auto SettingChanged = [](T val) {
    S = val;
};

static constexpr std::string unCamelCase(std::string str) {
    std::string out;
    out.reserve(str.size());
    for (const char c : str) {
        char lower = std::tolower(c);
        if (lower == c) {
            out.append(1, c);
        } else {
            out.append(1, '-');
            out.append(1, lower);
        }
    }
    return out;
}

static void settingsInit() {
    const auto M = Mod::get();
    //do you like my monstrosity? :)
    #define SETTING(type, name) \
        { \
            std::string settingName = unCamelCase(#name); \
            Settings::_ ## name = M->getSettingValue<type>(settingName); \
            listenForSettingChanges(settingName, SettingChanged<Settings::_ ## name>, M); \
        }

    SETTING(float, luaMaxExecutionTime);
    SETTING(int, luaInterruptResolution);
    SETTING(float, luaUninterruptibleGraceTime);

    SETTING(float, consoleFontSize);
    SETTING(float, consoleLineHeightMultiplier);

    #undef SETTING
}
$execute {
    settingsInit();
}