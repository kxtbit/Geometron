// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppCStyleCast
#include <generator>

#include <Geode/Geode.hpp>

#include "LuaEngine.hpp"

#include <string>

using namespace geode::prelude;
using namespace std::string_literals;

sol::object wrapGameObject(sol::state_view& lua, GameObject* object) {
    return sol::make_object(lua, std::move(Ref(object)));
}
sol::table wrapArrayOfGameObjects(sol::state_view& lua, CCArray* objects) {
    auto output = lua.create_table(objects->count());

    for (int i = 0; i < objects->count(); i++) {
        output.set(i + 1, wrapGameObject(lua, static_cast<GameObject*>(objects->objectAtIndex(i))));
    }
    return output;
}

void updateObjectPosition(EditorUI* self, GameObject* object) {
    auto layer = self->m_editorLayer;

    //object->quickUpdatePosition();
    object->updateStartValues();
    layer->removeObjectFromSection(object);
    layer->addToSection(object);

    bool speedObjectsUpdated = false;
    switch (object->m_objectID) {
        default: break;
        case 899: //color trigger, + legacy color triggers
        case 29: //BG
        case 30: //GRND
        case 105: //OBJ
        case 744: //3DL
        case 900: //GRND 2
        case 915: //LINE
            layer->m_colorTriggersChanged = true;
            break;
        case 1007: //alpha trigger
            layer->m_alphaTriggersChanged = true;
            break;
        case 1006: //pulse trigger
            layer->m_pulseTriggersChanged = true;
            break;
        case 741: //old teleport portal
            if (auto casted = typeinfo_cast<TeleportPortalObject*>(object)) {
                layer->removeObjectFromSection(casted->m_orangePortal);
                layer->addToSection(casted->m_orangePortal);
            }
            break;
            //i think this section is for various objects that change the flow of gameplay?
        case 200: //slow speed portal
        case 201: //1x speed portal
        case 202: //2x speed portal
        case 203: //3x speed portal
        case 1334: //4x speed portal
        case 1917: //reverse trigger
        case 1935: //timewarp trigger
        case 2900: //rotate gameplay trigger / arrow trigger
        case 2902: //new teleport portal
        case 3022: //teleport trigger
        case 3027: //teleport orb
            speedObjectsUpdated = true;
            break;
            //there is supposed to be something for the legacy enter effects but i can't understand the decompilation :(
    }
    if (object->m_isSpawnOrderTrigger)
        layer->m_spawnOrderObjectsChanged = true;
    if (object->m_dontIgnoreDuration) if (auto casted = typeinfo_cast<EffectGameObject*>(object))
        casted->m_endPosition = CCPoint {0, 0};
    if (speedObjectsUpdated || object->canReverse())
        self->m_speedObjectsUpdated = true;
}

std::string escapeString(const std::string& str) {
    std::stringstream out(str);
    out.clear();

    for (char c : str) {
        if (std::isprint(static_cast<unsigned char>(c))) {
            out << c;
        } else switch (c) {
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\\': out << "\\\\"; break;
            default: out << "\\x"s << std::hex << static_cast<unsigned int>(static_cast<unsigned char>(c)) << std::dec;
        }
    }
    return out.str();
}

void addProtectedMT(sol::state_view& lua, std::string_view name) {
    lua_State* L = lua.lua_state();

    //log::info("protect MT {}", name);
    lua_pushlstring(L, name.data(), name.size());
    lua_rawget(L, LUA_REGISTRYINDEX);
    pushConstantString<"__metatable">(L);
    lua_pushlstring(L, protectedMTMessage.data(), protectedMTMessage.size());
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

template<class F>
struct YieldingFunctionData {
    sol::variadic_args args;
    int startOfParams;
    YieldingFunctionCoroutine co;
    F function;
};
template<class F>
static int yieldingFunctionContinuation(lua_State* L, int status, lua_KContext context) {
    auto* data = static_cast<YieldingFunctionData<F>*>(lua_touserdata(L, lua_upvalueindex(1)));

    data->args = sol::variadic_args(L, data->startOfParams, lua_gettop(L));

    data->co.resume();
    auto& promise = data->co.promise();
    int numReturns = promise.stackEnd - promise.stackStart;
    data->startOfParams = promise.stackStart;
    if (data->co.done()) {
        //log::info("returning yielding function with {} values", numReturns);
        return numReturns;
    } else {
        //log::info("yielding from yielding function with {} values", numReturns);
        return lua_yieldk(L, numReturns, 0, &yieldingFunctionContinuation<F>);
    }
}
template<class F>
static int yieldingFunctionStub(lua_State* L) {
    auto* data = static_cast<YieldingFunctionData<F>*>(lua_touserdata(L, lua_upvalueindex(1)));

    data->args = sol::variadic_args(L, 1, lua_gettop(L));
    data->startOfParams = 1;
    data->co = data->function((sol::variadic_args&) data->args, L);

    data->co.resume();
    auto& promise = data->co.promise();
    int numReturns = promise.stackEnd - promise.stackStart;
    data->startOfParams = promise.stackStart;
    if (data->co.done()) {
        //log::info("returning yielding function with {} values", numReturns);
        return numReturns;
    } else {
        //log::info("yielding from yielding function with {} values", numReturns);
        return lua_yieldk(L, numReturns, 0, &yieldingFunctionContinuation<F>);
    }
}

template<class F>
sol::function yieldingFunction(sol::state_view& lua, F func) {
    auto L = lua.lua_state();
    lua_checkstack(L, 2);
    auto data = static_cast<YieldingFunctionData<F>*>(lua_newuserdatauv(L, sizeof(YieldingFunctionData<F>), 0));
    lua_pushcclosure(L, &yieldingFunctionStub<F>, 1);
    data->function = func;

    return sol::stack::get<sol::function>(L, lua_absindex(L, -1));
}

//template int yieldingFunctionContinuation<YieldingFunctionCoroutine (*)(sol::variadic_args, curengine)>(lua_State*, int, lua_KContext);
template sol::function yieldingFunction(sol::state_view& lua, YieldingFunctionCoroutine (*)(sol::variadic_args, lua_State*));