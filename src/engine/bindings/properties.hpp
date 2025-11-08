#pragma once

#ifndef GEOMETRON_PROPERTIES_HPP
#define GEOMETRON_PROPERTIES_HPP

#include <Geode/Geode.hpp>

#include "../LuaEngine.hpp"
#include "../LuaUtils.hpp"
#include "../../utils/types/GPoint.hpp"

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

template<class O>
struct object_subclass_name {};
//i might be getting a bit overzealous with this metaprogramming
#define SC_REG(subclass) template<> struct object_subclass_name<subclass> { static constexpr FixedString v = #subclass; }
SC_REG(AdvancedFollowEditObject);
SC_REG(AdvancedFollowTriggerObject);
SC_REG(AnimatedGameObject);
SC_REG(ArtTriggerGameObject);
SC_REG(AudioLineGuideGameObject);
SC_REG(CameraTriggerGameObject);
SC_REG(ChanceTriggerGameObject);
SC_REG(CheckpointGameObject);
SC_REG(CountTriggerGameObject);
SC_REG(DashRingObject);
SC_REG(EffectGameObject);
SC_REG(EndPortalObject);
SC_REG(EndTriggerGameObject);
SC_REG(EnhancedGameObject);
SC_REG(EnhancedTriggerObject);
SC_REG(EnterEffectObject);
SC_REG(EventLinkTrigger);
SC_REG(ForceBlockGameObject);
SC_REG(GameOptionsTrigger);
SC_REG(GradientTriggerObject);
SC_REG(ItemTriggerGameObject);
SC_REG(KeyframeAnimTriggerObject);
SC_REG(KeyframeGameObject);
SC_REG(LabelGameObject);
SC_REG(ObjectControlGameObject);
SC_REG(ParticleGameObject);
SC_REG(PlayerControlGameObject);
SC_REG(PlayerObject);
SC_REG(RandTriggerGameObject);
SC_REG(RingObject);
SC_REG(RotateGameplayGameObject);
SC_REG(SequenceTriggerGameObject);
SC_REG(SFXTriggerGameObject);
SC_REG(ShaderGameObject);
SC_REG(SmartGameObject);
SC_REG(SongTriggerGameObject);
SC_REG(SpawnParticleGameObject);
SC_REG(SpawnTriggerGameObject);
SC_REG(SpecialAnimGameObject);
SC_REG(StartPosObject);
SC_REG(TeleportPortalObject);
SC_REG(TextGameObject);
SC_REG(TimerTriggerGameObject);
SC_REG(TransformTriggerGameObject);
SC_REG(TriggerControlGameObject);
SC_REG(UISettingsGameObject);
#undef SC_REG

template<class O>
static O* subclassCast(GameObject* object, lua_State* L) {
    if (O* sub = typeinfo_cast<O*>(object)) {
        return sub;
    } else {
        constexpr FixedString errStr(FixedString("object is not a subclass of "), object_subclass_name<O>::v);
        luaL_where(L, 1);
        pushConstantString<errStr>(L);
        lua_concat(L, 2);
        lua_error(L);
        return nullptr; //this line should never execute
    }
}
template<auto Prop, auto WriteHook>
static constexpr auto gameObjectSubclassProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(!std::is_same_v<CCPoint, T>, "must not be a CCPoint");
    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
        return sol::make_object(lua, subclassCast<O>(object, lua)->*Prop);
    }, [](GameObject* object, T value, sol::this_state lua) {
        O* effect = subclassCast<O>(object, lua);
        effect->*Prop = value;
        WriteHook(effect, lua);
    });
}
template<auto Prop>
static constexpr auto gameObjectSubclassProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(!std::is_same_v<CCPoint, T>, "must not be a CCPoint");
    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
        return sol::make_object(lua, subclassCast<O>(object, lua)->*Prop);
    }, [](GameObject* object, T value, sol::this_state lua) {
        subclassCast<O>(object, lua)->*Prop = value;
    });
}
template<auto Prop>
static constexpr auto gameObjectSubclassReadOnlyProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(!std::is_same_v<CCPoint, T>, "must not be a CCPoint");
    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::readonly_property([](GameObject* object, sol::this_state lua) -> sol::object {
        return sol::make_object(lua, subclassCast<O>(object, lua)->*Prop);
    });
}

template<auto Prop>
static constexpr auto gameObjectSubclassPointProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(std::is_same_v<CCPoint, T>, "must be a CCPoint");
    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
        auto point = subclassCast<O>(object, lua)->*Prop;
        return sol::make_object(lua, GPoint{point.x, point.y});
    }, [](GameObject* object, GPoint value, sol::this_state lua) {
        subclassCast<O>(object, lua)->*Prop = CCPoint{static_cast<float>(value.x), static_cast<float>(value.y)};
    });
}
template<auto Prop>
static constexpr auto gameObjectSubclassReadOnlyPointProperty() {
    using O = member_pointer_base<decltype(Prop)>::type;
    using T = member_pointer_target<decltype(Prop)>::type;

    static_assert(std::is_same_v<CCPoint, T>, "must be a CCPoint");
    static_assert(std::is_base_of_v<GameObject, O>, "must be a subclass of GameObject");

    return sol::readonly_property([](GameObject* object, sol::this_state lua) -> sol::object {
        auto point = subclassCast<O>(object, lua)->*Prop;
        return sol::make_object(lua, GPoint{point.x, point.y});
    });
}

void addPropertiesForGameObjects(sol::state_view& lua, EditorUI* self, sol::usertype<GameObject> gameObjectType);
void addPropertiesForTriggers(sol::state_view& lua, EditorUI* self, sol::usertype<GameObject> gameObjectType);

#define WriteHook(T, N, ...) struct N { static void f(T* object, sol::this_state lua) __VA_ARGS__ }

#endif //GEOMETRON_PROPERTIES_HPP