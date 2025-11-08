// ReSharper disable CppLocalVariableMayBeConst
#include "LuaModules.hpp"

#include <sol/sol.hpp>

#include "LuaUtils.hpp"

static constexpr FixedString SMARTARRAY_MT_KEY = "smartarray_mt";

static bool metatableEquals(lua_State* L, int object, int mt) {
    if (!lua_getmetatable(L, object)) return false;
    bool equal = lua_rawequal(L, mt, -1);
    lua_pop(L, 1);
    return equal;
}

template<FixedString Op>
static int smartArrayMetamethod(lua_State* L) {
    {
        int nArgs = lua_gettop(L);
        //nArgs * 3 should probably be enough idk
        luaL_checkstack(L, nArgs * 3, "out of stack space");
        std::vector<int> smartArrays(nArgs);

        pushConstantString<SMARTARRAY_MT_KEY>(L);
        lua_gettable(L, LUA_REGISTRYINDEX);
        int mt = lua_gettop(L);
        lua_Unsigned lastSize = -1;
        for (int i = 1; i <= nArgs; i++) {
            if (lua_istable(L, i) && metatableEquals(L, i, mt)) {
                if (lua_Unsigned thisSize = lua_rawlen(L, i); lastSize != -1 && thisSize != lastSize) {
                    pushConstantString<"smart arrays must have identical size">(L);
                    goto error;
                } else lastSize = thisSize;
                smartArrays.push_back(i);
                lua_pushvalue(L, i);
            }
        }
        int numSmartArrays = smartArrays.size();
        if (numSmartArrays == 0) {
            pushConstantString<"smart array operation called with no smart arrays">(L);
            goto error;
        } else if (lastSize == 0) {
            pushConstantString<"smart arrays are empty">(L);
            goto error;
        }
        pushConstantString<Op>(L);
        //place smart arrays at beginning
        lua_rotate(L, 1, numSmartArrays);
        int argsStart = numSmartArrays + 1;

        for (lua_Unsigned i = 1; i <= lastSize; i++) {

        }
    }
    //jump out of scope on error to make sure destructors get called
    error:
    luaL_where(L, 1);
    lua_insert(L, -2);
    lua_concat(L, 2);
    return lua_error(L);
}

void luaAddSmartArray(sol::state_view& lua) {
    lua_State* L = lua.lua_state();

    lua_createtable(L, 0, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, SMARTARRAY_MT_KEY.v);
}
