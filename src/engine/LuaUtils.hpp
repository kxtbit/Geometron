#pragma once

#ifndef GEOMETRON_LUAUTILS_HPP
#define GEOMETRON_LUAUTILS_HPP

#include <Geode/Geode.hpp>
#include <sol/sol.hpp>

#include "../utils/types/GPoint.hpp"

using namespace geode::prelude;

template<size_t L>
struct FixedString {
    static constexpr size_t len = L;

    char v[L + 1] = {};
    constexpr FixedString(const char (&s)[L + 1]) {
        std::copy_n(s, L + 1, v);
    }
    template<size_t LA, size_t LB>
    constexpr FixedString(FixedString<LA> a, FixedString<LB> b) {
        std::copy_n(a.v, LA, v);
        std::copy_n(b.v, LB + 1, v + LA);
    }
};
template<size_t L>
FixedString(const char(&s)[L]) -> FixedString<L - 1>;
template<size_t LA, size_t LB>
FixedString(FixedString<LA>, FixedString<LB>) -> FixedString<LA + LB>;

template<FixedString str>
void pushConstantString(lua_State* L) {
    lua_pushlstring(L, str.v, str.len);
}
inline std::string_view checkStringView(lua_State* L, int index) {
    size_t len;
    auto cstr = luaL_checklstring(L, index, &len);
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return std::string_view(cstr, len);
}

bool gameObjectExists(GameObject* object);

//might call lua_error, make sure no objects are leaked
inline void checkObjectExists(lua_State* L, GameObject* object) {
    if (!gameObjectExists(object))
        luaL_error(L, "object does not exist");
}
sol::object wrapGameObject(lua_State* L, GameObject* object);
sol::table wrapArrayOfGameObjects(lua_State* L, CCArray* objects);

void updateObjectPosition(EditorUI* editor, GameObject* object);
void updateEditorSelection(EditorUI* editor);
void purgeObjectFromUndoHistory(EditorUI* editor, CCArray* history, GameObject* object);
void prepareToDeleteObject(EditorUI* editor, GameObject* object);
GameObject* safeCreateObject(EditorUI* editor, int id, GPoint pos);
void safeDeleteObject(EditorUI* editor, GameObject* object);

GPoint getEditorViewCenter(EditorUI* editor);
void setEditorViewCenter(EditorUI* editor, GPoint pos);

std::string getCurrentFontName();
CCBMFontConfiguration* getCurrentFontConfiguration();
CCArrayExt<TextGameObject> splitText(EditorUI* editor, TextGameObject* text);

void addProtectedMT(sol::state_view& lua, std::string_view name);
inline static const std::string protectedMTMessage = "this metatable is protected";
template<class T>
void setupImmutableMT(sol::state_view& lua, auto object, sol::usertype<T> ut) {
    if constexpr (!std::is_same_v<decltype(object), std::nullptr_t>) {
        sol::optional<sol::table> omt = object[sol::metatable_key];
        auto mt = omt.has_value() ? omt.value() : lua.create_table(0, 2);
        mt["__newindex"] = sol::make_object(lua, sol::as_function([](lua_State* L) -> int {
            return luaL_error(L, "cannot change this object");
        }));
        mt["__metatable"] = protectedMTMessage;
        if (!omt.has_value()) object[sol::metatable_key] = mt;
    }

    //there is probably a better way to do this part but i am too dumb
    luaL_checkstack(lua.lua_state(), 3, "not enough stack space");
    addProtectedMT(lua, sol::usertype_traits<T>::metatable());
    addProtectedMT(lua, sol::usertype_traits<T>::user_metatable());
    addProtectedMT(lua, sol::usertype_traits<const T>::metatable());
    addProtectedMT(lua, sol::usertype_traits<sol::d::u<T>>::metatable());
    addProtectedMT(lua, sol::usertype_traits<T*>::metatable());
    addProtectedMT(lua, sol::usertype_traits<T const*>::metatable());
}

#endif //GEOMETRON_LUAUTILS_HPP