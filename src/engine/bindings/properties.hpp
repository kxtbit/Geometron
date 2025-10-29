#pragma once

#ifndef GEOMETRON_PROPERTIES_HPP
#define GEOMETRON_PROPERTIES_HPP

#include <Geode/Geode.hpp>

#include "../LuaEngine.hpp"

using namespace geode::prelude;

template<class P>
struct member_pointer_base {
    template<class O, class T>
    static O extract(T O::*);

    using type = decltype(extract(std::declval<P>()));
};
template<class P>
struct member_pointer_target {
    template<class O, class T>
    static T extract(T O::*);

    using type = decltype(extract(std::declval<P>()));
};

template<auto Prop, auto WriteHook>
static constexpr auto gameObjectSubclassProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
        if (O* effect = typeinfo_cast<O*>(object)) {
            return sol::make_object(lua, effect->*Prop);
        }
        return sol::nil;
    }, [](GameObject* object, T value, sol::this_state lua) {
        if (O* effect = typeinfo_cast<O*>(object)) {
            effect->*Prop = value;
            WriteHook(effect, lua);
        }
    });
}
template<auto Prop>
static constexpr auto gameObjectSubclassProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
        if (O* effect = typeinfo_cast<O*>(object)) {
            return sol::make_object(lua, effect->*Prop);
        }
        return sol::nil;
    }, [](GameObject* object, T value) {
        if (O* effect = typeinfo_cast<O*>(object))
            effect->*Prop = value;
    });
}
template<auto Prop>
static constexpr auto gameObjectSubclassReadOnlyProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::readonly_property([](GameObject* object, sol::this_state lua) -> sol::object {
        if (O* effect = typeinfo_cast<O*>(object)) {
            return sol::make_object(lua, effect->*Prop);
        }
        return sol::nil;
    });
}

template<auto Prop>
static constexpr auto gameObjectSubclassPointProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(std::is_same_v<CCPoint, T>, "must be a CCPoint");
    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
        if (O* effect = typeinfo_cast<O*>(object)) {
            auto point = effect->*Prop;
            return sol::make_object(lua, LuaPoint{point.x, point.y});
        }
        return sol::nil;
    }, [](GameObject* object, LuaPoint value) {
        if (O* effect = typeinfo_cast<O*>(object))
            effect->*Prop = CCPoint{static_cast<float>(value.x), static_cast<float>(value.y)};
    });
}
template<auto Prop>
static constexpr auto gameObjectSubclassReadOnlyPointProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(std::is_same_v<CCPoint, T>, "must be a CCPoint");
    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::readonly_property([](GameObject* object, sol::this_state lua) -> sol::object {
        if (O* effect = typeinfo_cast<O*>(object)) {
            auto point = effect->*Prop;
            return sol::make_object(lua, LuaPoint{point.x, point.y});
        }
        return sol::nil;
    });
}

void addPropertiesForGameObjects(sol::state_view& lua, EditorUI* self, sol::usertype<GameObject> gameObjectType);
void addPropertiesForTriggers(sol::state_view& lua, EditorUI* self, sol::usertype<GameObject> gameObjectType);

#define WriteHook(T, N, ...) struct N { static void f(T* object, sol::this_state lua) __VA_ARGS__ }

#endif //GEOMETRON_PROPERTIES_HPP