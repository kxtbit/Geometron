// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#pragma once

#ifndef GEOMETRON_SETTINGS_HPP
#define GEOMETRON_SETTINGS_HPP

#ifdef SETTINGS_IMPL
#include <Geode/Geode.hpp>
using namespace geode::prelude;

template<auto& S>
static constexpr auto SettingChanged = [](std::remove_reference_t<decltype(S)> val) {
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

#define DEFINE_SETTING(type, name) \
    static type _##name; \
    namespace Settings { type name() {return _##name;} }; \
    $execute { \
        Mod* M = Mod::get(); \
        std::string settingName = unCamelCase(#name); \
        _##name = M->getSettingValue<type>(settingName); \
        listenForSettingChanges<type>(settingName, SettingChanged<_##name>, M); \
    }

#else
#define DEFINE_SETTING(type, name) \
    namespace Settings { type name(); }
#endif

DEFINE_SETTING(bool, interfaceTransparentScriptMenu);

DEFINE_SETTING(float, luaMaxExecutionTime);
DEFINE_SETTING(int, luaInterruptResolution);
DEFINE_SETTING(float, luaUninterruptibleGraceTime);
DEFINE_SETTING(bool, luaDisablePreemptionSystem)
DEFINE_SETTING(bool, luaAllowDebugLibrary);
DEFINE_SETTING(bool, luaAllowBytecodeLoading);

DEFINE_SETTING(float, consoleFontSize);
DEFINE_SETTING(float, consoleLineHeightMultiplier);
DEFINE_SETTING(bool, consoleAutoFocusInput);

#endif //GEOMETRON_SETTINGS_HPP