#pragma once

#ifndef GEOMETRON_SETTINGS_HPP
#define GEOMETRON_SETTINGS_HPP

class Settings {
    static float _luaMaxExecutionTime;
    static int _luaInterruptResolution;
    static float _luaUninterruptibleGraceTime;

    static float _consoleFontSize;
    static float _consoleLineHeightMultiplier;

    friend void settingsInit();
public:
    static float luaMaxExecutionTime() {return _luaMaxExecutionTime;}
    static int luaInterruptResolution() {return _luaInterruptResolution;}
    static float luaUninterruptibleGraceTime() {return _luaUninterruptibleGraceTime;}

    static float consoleFontSize() {return _consoleFontSize;}
    static float consoleLineHeightMultiplier() {return _consoleLineHeightMultiplier;}
};

#endif //GEOMETRON_SETTINGS_HPP