#pragma once

#ifndef GEOMETRON_LUAMODULES_HPP
#define GEOMETRON_LUAMODULES_HPP

#include "LuaBackend.hpp"

class EditorUI;

void luaAddIO(sol::state_view& lua);
void luaAddEnums(sol::state_view& lua);
void luaAddUsertypes(sol::state_view& lua, EditorUI* self);
void luaAddSmartArray(sol::state_view& lua);

#endif //GEOMETRON_LUAMODULES_HPP