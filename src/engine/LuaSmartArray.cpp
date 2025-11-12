// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppDeclarationHidesUncapturedLocal
#include "LuaModules.hpp"

#include <sol/sol.hpp>

#include "LuaUtils.hpp"

static constexpr FixedString SMARTARRAY_MT_KEY = "smartarray_mt";

template<FixedString S>
static void initBufferWithConstantString(lua_State* L, luaL_Buffer* buf) {
    luaL_buffinitsize(L, buf, S.len);
    luaL_addlstring(buf, S.v, S.len);
}

static int tailCallContinuation(lua_State* L, int, lua_KContext) {
    return lua_gettop(L);
}
static int tailCall(lua_State* L, int nArgs) {
    int usefulValues = nArgs + 1;
    if (int top = lua_gettop(L); top > usefulValues) {
        int argsStart = top - usefulValues + 1;
        for (int i = 1; i <= usefulValues; i++) {
            lua_copy(L, argsStart + i - 1, i);
        }
        lua_settop(L, usefulValues);
    }
    lua_callk(L, nArgs, LUA_MULTRET, 0, &tailCallContinuation);
    return tailCallContinuation(L, LUA_OK, 0);
}

static bool metatableEquals(lua_State* L, int object, int mt) {
    if (!lua_getmetatable(L, object)) return false;
    bool equal = lua_rawequal(L, mt, -1);
    lua_pop(L, 1);
    return equal;
}
//this assumes upvalue 1 is the expected metatable
static bool objectIsSmartArray(lua_State* L, int object) {
    return lua_type(L, object) == LUA_TTABLE && metatableEquals(L, object, lua_upvalueindex(1));
}
static bool getMetatableOfFirst(lua_State* L, int index) {
    lua_rawgeti(L, index, 1);
    if (lua_getmetatable(L, -1)) {
        lua_remove(L, -2);
        return true;
    } else {
        lua_pop(L, 1);
        return false;
    }
}
template<FixedString MM>
static bool getMetamethodOfFirst(lua_State* L, int index) {
    if (getMetatableOfFirst(L, index)) {
        pushConstantString<MM>(L);
        if (lua_rawget(L, -2) != LUA_TNIL) {
            lua_remove(L, -2);
            return true;
        } else {
            lua_pop(L, 2);
            return false;
        }
    }
    return false;
}

[[noreturn]]
static void defaultErrorThrow(lua_State* L) {
    luaL_where(L, 1);
    lua_insert(L, -2);
    lua_concat(L, 2);
    lua_error(L);

    //this should never happen but whatever
    throw 0;
}

template<FixedString Begin>
static void badLengthMessage(lua_State* L, lua_Unsigned len, lua_Unsigned len2) {
    constexpr FixedString header(Begin, " ("_f);
    lua_settop(L, 0);
    if (!lua_checkstack(L, 5)) {
        pushConstantString<"not enough stack space">(L);
        return;
    }
    pushConstantString<header>(L);
    lua_pushinteger(L, len);
    pushConstantString<" != ">(L);
    lua_pushinteger(L, len2);
    pushConstantString<")">(L);
    lua_concat(L, 5);
}

template<FixedString MM, int OpID>
static int binarySmartArrayMetamethod(lua_State* L) {
    {
        lua_settop(L, 2);

        //pushConstantString<SMARTARRAY_MT_KEY>(L);
        //lua_gettable(L, LUA_REGISTRYINDEX);
        constexpr int mt = lua_upvalueindex(1);//lua_gettop(L);

        bool s1 = objectIsSmartArray(L, 1), s2 = objectIsSmartArray(L, 2);
        if (!(s1 || s2)) {
            lua_settop(L, 0);
            pushConstantString<"smart array binary operation requires at least one smart array">(L);
            goto error;
        }
        int firstSmartArray = s1 ? 1 : 2;
        lua_Unsigned len = lua_rawlen(L, firstSmartArray);
        if (lua_Unsigned len2; s1 && s2 && (len2 = lua_rawlen(L, 2)) != len) {
            badLengthMessage<"smart array binary operation arguments must be the same length">(L, len, len2);
            goto error;
        }
        if (len == 0) {
            lua_newtable(L);
            lua_pushvalue(L, mt);
            lua_setmetatable(L, -2);
            return 1;
        }
        int mm = -1;
        if ((s1 && getMetamethodOfFirst<MM>(L, 1)) || (s2 && getMetamethodOfFirst<MM>(L, 2))) {
            //it is assumed that the objects in a smart array have the same metamethods
            mm = lua_gettop(L);
        }

        lua_createtable(L, len, 0);
        int out = lua_gettop(L);
        for (lua_Unsigned i = 1; i <= len; i++) {
            if (mm != -1)
                lua_pushvalue(L, mm);

            if (s1) {
                lua_rawgeti(L, 1, i);
            } else lua_pushvalue(L, 1);
            if (s2) {
                lua_rawgeti(L, 2, i);
            } else lua_pushvalue(L, 2);

            if (mm != -1) {
                lua_call(L, 2, 1);
                lua_rawseti(L, out, i);
            } else {
                lua_arith(L, OpID);
                lua_rawseti(L, out, i);
            }
        }
        lua_settop(L, out);
        lua_pushvalue(L, mt);
        lua_setmetatable(L, out);
        return 1;
    }
    //jump out of scope on error to make sure destructors get called
    error:
    defaultErrorThrow(L);
}

template<size_t NameL>
struct SmartArrayMetamethod {
    FixedString<NameL> name;
    lua_CFunction func;
};
template<size_t NameL>
SmartArrayMetamethod(FixedString<NameL>, lua_CFunction) -> SmartArrayMetamethod<NameL>;

template<FixedString MM, int OpID>
struct BinarySmartArrayMetamethod_struct {
    static constexpr SmartArrayMetamethod<MM.len> v = {MM, &binarySmartArrayMetamethod<MM, OpID>};
};
template<FixedString MM, int OpID>
constexpr auto BinarySmartArrayMetamethod = BinarySmartArrayMetamethod_struct<MM, OpID>::v;

template<SmartArrayMetamethod MM>
static void addSmartArrayMetamethod(lua_State* L) {
    pushConstantString<MM.name>(L);
    //push a copy of the metatable to the stack to use as upvalue
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, MM.func, 1);
    lua_rawset(L, -3);
}

int smartArrayCallHandler(lua_State* L) {
    {
        constexpr int mt = lua_upvalueindex(1);

        int nAllArgs = lua_gettop(L);
        int nFuncArgs = nAllArgs - 1;
        //idk that's probably enough right?
        luaL_checkstack(L, nAllArgs * 2 + 8, "not enough stack space");

        luaL_checktype(L, 1, LUA_TTABLE);
        lua_Unsigned len = lua_rawlen(L, 1);
        //avoid vector<bool>
        bool isSmartArray[nFuncArgs];
        for (int i = 1; i <= nFuncArgs; i++) {
            if (objectIsSmartArray(L, i + 1)) {
                isSmartArray[i - 1] = true;
                if (lua_Unsigned len2 = lua_rawlen(L, i + 1); len2 != len) {
                    badLengthMessage<"smart array call arguments must be the same length as base">(L, len, len2);
                    goto error;
                }
            }
        }

        lua_createtable(L, len, 0);
        int out = lua_gettop(L);

        for (lua_Unsigned i = 1; i <= len; i++) {
            lua_rawgeti(L, 1, i);
            for (int j = 1; j <= nFuncArgs; j++) {
                if (isSmartArray[j - 1]) {
                    lua_rawgeti(L, j + 1, i);
                } else {
                    lua_pushvalue(L, j + 1);
                }
            }

            lua_call(L, nFuncArgs, 1);
            lua_rawseti(L, out, i);
        }

        lua_settop(L, out);
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_setmetatable(L, out);
        return 1;
    }
    error:
    defaultErrorThrow(L);
}
int smartArrayStringIndexHandler(lua_State* L, lua_Unsigned len) {
    size_t strLen;
    auto str = lua_tolstring(L, 2, &strLen);
    if (strLen >= 1) {
        int op;
        switch (str[0]) {
            case '+':
                op = LUA_OPADD;
                break;
            case '*':
                op = LUA_OPMUL;
                break;
            case '&':
                op = LUA_OPBAND;
                break;
            case '|':
                op = LUA_OPBOR;
                break;
            case '~':
                op = LUA_OPBXOR;
                break;
            default: return -1;
        }

        if (len == 0) {
            lua_pushnil(L);
            return 1;
        }
        lua_rawgeti(L, 1, 1);
        for (lua_Unsigned i = 2; i <= len; i++) {
            lua_rawgeti(L, 1, i);
            lua_arith(L, op);
        }
        if (strLen >= 2 && str[1] == '/') {
            lua_pushinteger(L, len);
            lua_arith(L, LUA_OPDIV);
        }
        return 1;
    }
    return -1;
}

void luaAddSmartArray(sol::state_view& lua) {
    lua_State* L = lua.lua_state();

    luaL_checkstack(L, 8, "stack overflow");

    lua_createtable(L, 0, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, SMARTARRAY_MT_KEY.v);

    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__add", LUA_OPADD>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__sub", LUA_OPSUB>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__mul", LUA_OPMUL>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__div", LUA_OPDIV>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__idiv", LUA_OPIDIV>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__mod", LUA_OPMOD>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__pow", LUA_OPPOW>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__band", LUA_OPBAND>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__bor", LUA_OPBOR>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__bxor", LUA_OPBXOR>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__shl", LUA_OPSHL>>(L);
    addSmartArrayMetamethod<BinarySmartArrayMetamethod<"__shr", LUA_OPSHR>>(L);

    addSmartArrayMetamethod<SmartArrayMetamethod {"__call"_f, &smartArrayCallHandler}>(L);

    addSmartArrayMetamethod<SmartArrayMetamethod {"__index"_f, [](lua_State* L) -> int {
        {
            constexpr int mt = lua_upvalueindex(1);

            luaL_checkstack(L, 5, "not enough stack space");

            lua_settop(L, 2);
            luaL_checktype(L, 1, LUA_TTABLE);
            lua_Unsigned len = lua_rawlen(L, 1);
            switch (lua_type(L, 2)) {
                case LUA_TSTRING: {
                    if (int ret = smartArrayStringIndexHandler(L, len); ret >= 0) return ret;
                    break;
                }
                case LUA_TNUMBER: {
                    lua_rawget(L, 1);
                    return 1;
                }
                case LUA_TFUNCTION: {
                    lua_newtable(L);
                    int out = lua_gettop(L);

                    for (lua_Unsigned i = 1, j = 1; i <= len; i++) {
                        lua_pushvalue(L, 2);
                        lua_rawgeti(L, 1, i);
                        lua_call(L, 1, LUA_MULTRET);

                        int nRets = lua_gettop(L) - out;
                        for (lua_Unsigned k = j + nRets - 1; k >= j; k--)
                            lua_rawseti(L, out, k);
                        j += nRets;
                    }
                    lua_settop(L, out);
                    lua_pushvalue(L, lua_upvalueindex(1));
                    lua_setmetatable(L, out);
                    return 1;
                }
                default: break;
            }

            bool smartKey = objectIsSmartArray(L, 2);
            if (lua_Unsigned len2; smartKey && (len2 = lua_rawlen(L, 2)) != len) {
                badLengthMessage<"smart array key in index opration must be the same length as base">(L, len, len2);
                goto error;
            }

            lua_createtable(L, len, 0);
            int out = lua_gettop(L);

            for (lua_Unsigned i = 1; i <= len; i++) {
                lua_rawgeti(L, 1, i);
                if (smartKey) {
                    lua_rawgeti(L, 2, i);
                } else
                    lua_pushvalue(L, 2);
                lua_gettable(L, -2);
                lua_rawseti(L, out, i);
                lua_pop(L, 1);
            }
            lua_settop(L, out);
            lua_pushvalue(L, lua_upvalueindex(1));
            lua_setmetatable(L, out);
            return 1;
        }
        error:
        defaultErrorThrow(L);
    }}>(L);
    addSmartArrayMetamethod<SmartArrayMetamethod {"__newindex"_f, [](lua_State* L) -> int {
        {
            constexpr int mt = lua_upvalueindex(1);

            luaL_checkstack(L, 6, "not enough stack space");

            lua_settop(L, 3);
            luaL_checktype(L, 1, LUA_TTABLE);
            if (lua_type(L, 2) == LUA_TNUMBER) {
                lua_rawset(L, 1);
                return 0;
            }
            lua_Unsigned len = lua_rawlen(L, 1);

            bool smartKey = objectIsSmartArray(L, 2);
            if (lua_Unsigned len2; smartKey && (len2 = lua_rawlen(L, 2)) != len) {
                badLengthMessage<"smart array key in index write operation must be the same length as base">(L, len, len2);
                goto error;
            }
            bool smartValue = objectIsSmartArray(L, 3);
            if (lua_Unsigned len2; smartValue && (len2 = lua_rawlen(L, 3)) != len) {
                badLengthMessage<"smart array value in index write operation must be the same length as base">(L, len, len2);
                goto error;
            }

            for (lua_Unsigned i = 1; i <= len; i++) {
                lua_rawgeti(L, 1, i);
                if (smartKey) {
                    lua_rawgeti(L, 2, i);
                } else
                    lua_pushvalue(L, 2);
                if (smartValue) {
                    lua_rawgeti(L, 3, i);
                } else
                    lua_pushvalue(L, 3);
                lua_settable(L, -3);
                lua_pop(L, 1);
            }
            return 0;
        }
        error:
        defaultErrorThrow(L);
    }}>(L);

    addSmartArrayMetamethod<SmartArrayMetamethod {FixedString("__tostring"), [](lua_State* L) -> int {
        lua_settop(L, 1);
        luaL_checktype(L, 1, LUA_TTABLE);

        lua_Unsigned len = lua_rawlen(L, 1);

        luaL_Buffer out;
        initBufferWithConstantString<"smart array {">(L, &out);

        size_t headerLen = luaL_bufflen(&out);
        size_t maxDataLen = 32;
        for (lua_Unsigned i = 1; i <= len; i++) {
            lua_rawgeti(L, 1, i);
            luaL_tolstring(L, -1, nullptr);
            luaL_addvalue(&out);
            lua_pop(L, 1);
            if (i < len) bufferAddConstantString<", ">(&out);

            if (luaL_bufflen(&out) - headerLen > maxDataLen) {
                out.n = headerLen + maxDataLen;
                bufferAddConstantString<"...">(&out);
                break;
            }
        }
        bufferAddConstantString<"}">(&out);
        luaL_pushresult(&out);
        return 1;
    }}>(L);

    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    pushConstantString<"smart">(L);
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, [](lua_State* L) -> int {
        {
            switch (lua_type(L, 1)) {
                case LUA_TTABLE: {
                    lua_settop(L, 1);
                    lua_pushvalue(L, lua_upvalueindex(1));
                    lua_setmetatable(L, 1);
                    return 1;
                }
                case LUA_TFUNCTION: {
                    int nAllArgs = lua_gettop(L);
                    luaL_checkstack(L, nAllArgs * 2 + 8, "not enough stack space");

                    int nFuncArgs = nAllArgs - 1;

                    bool isSmartArray[nFuncArgs];
                    bool hasLen = false;
                    lua_Unsigned len = 0;
                    for (int i = 1; i <= nFuncArgs; i++) {
                        if (objectIsSmartArray(L, i + 1)) {
                            isSmartArray[i - 1] = true;
                            if (!hasLen) {
                                len = lua_rawlen(L, i + 1);
                                hasLen = true;
                            } else if (lua_Unsigned len2 = lua_rawlen(L, i + 1); len2 != len) {
                                badLengthMessage<"smart array call arguments must be same length">(L, len, len2);
                                goto error;
                            }
                        }
                    }

                    lua_createtable(L, len, 0);
                    int out = lua_gettop(L);

                    for (lua_Unsigned i = 1; i <= len; i++) {
                        lua_pushvalue(L, 1);
                        for (int j = 1; j <= nFuncArgs; j++) {
                            if (isSmartArray[j - 1]) {
                                lua_rawgeti(L, j + 1, i);
                            } else {
                                lua_pushvalue(L, j + 1);
                            }
                        }

                        lua_call(L, nFuncArgs, 1);
                        lua_rawseti(L, out, i);
                    }

                    lua_settop(L, out);
                    lua_pushvalue(L, lua_upvalueindex(1));
                    lua_setmetatable(L, out);
                    return 1;
                }
                default: {
                    pushConstantString<"first argument must be table or function">(L);
                }
            }
        }
        error:
        defaultErrorThrow(L);
    }, 1);
    lua_rawset(L, -3);
    lua_pop(L, 2);
}
