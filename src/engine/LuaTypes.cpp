// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppDeclarationHidesUncapturedLocal
#include <Geode/Geode.hpp>

#include "LuaEngine.hpp"
#include "bindings/properties.hpp"

#include <numbers>

using namespace geode::prelude;

double LuaPoint::dot(const LuaPoint other) const {
    return other.x * x + other.y * y;
}
double LuaPoint::magnitude() const {
    return std::sqrt(dot(*this));
}
LuaPoint LuaPoint::unit() const {
    double d = std::sqrt(dot(*this));
    return LuaPoint(x / d, y / d);
}

LuaPoint LuaPoint::operator+(const LuaPoint r) const {
    return {x + r.x, y + r.y};
}
LuaPoint LuaPoint::operator-(const LuaPoint r) const {
    return {x - r.x, y - r.y};
}
LuaPoint LuaPoint::operator*(const LuaPoint r) const {
    return {x * r.x, y * r.y};
}
LuaPoint LuaPoint::operator*(double r) const {
    return {x * r, y * r};
}
LuaPoint LuaPoint::operator/(const LuaPoint r) const {
    return {x / r.x, y / r.y};
}
LuaPoint LuaPoint::operator/(double r) const {
    return {x / r, y / r};
}

LuaPoint LuaPoint::operator-() const {
    return {-x, -y};
}

LuaPoint::operator std::string() const {
    return fmt::format("point {{{}, {}}}", x, y);
}

void luaAddEnums(sol::state_view& lua) {
    lua["EasingType"] = lua.create_table_with(
        "None", 0, 0, "None",
        "EaseInOut", 1, 1, "EaseInOut",
        "EaseIn", 2, 2, "EaseIn",
        "EaseOut", 3, 3, "EaseOut",
        "ElasticInOut", 4, 4, "ElasticInOut",
        "ElasticIn", 5, 5, "ElasticIn",
        "ElasticOut", 6, 6, "ElasticOut",
        "BounceInOut", 7, 7, "BounceInOut",
        "BounceIn", 8, 8, "BounceIn",
        "BounceOut", 9, 9, "BounceOut",
        "ExponentialInOut", 10, 10, "ExponentialInOut",
        "ExponentialIn", 11, 11, "ExponentialIn",
        "ExponentialOut", 12, 12, "ExponentialOut",
        "SineInOut", 13, 13, "SineInOut",
        "SineIn", 14, 14, "SineIn",
        "SineOut", 15, 15, "SineOut",
        "BackInOut", 16, 16, "BackInOut",
        "BackIn", 17, 17, "BackIn",
        "BackOut", 18, 18, "BackOut");
    lua["ZLayer"] = lua.create_table_with(
        "B5", -5, -5, "B5",
        "B4", -3, -3, "B4",
        "B3", -1, -1, "B3",
        "B2", 1, 1, "B2",
        "B1", 3, 3, "B1",
        "Default", 0, 0, "Default",
        "T1", 5, 5, "T1",
        "T2", 7, 7, "T2",
        "T3", 9, 9, "T3",
        "T4", 11, 11, "T4");
    lua["GameObjectClassType"] = lua.create_table_with(
        "Game", 0, 0, "Game",
        "Effect", 1, 1, "Effect",
        "Animated", 2, 2, "Animated",
        "Enhanced", 4, 4, "Enhanced",
        "Smart", 5, 5, "Smart");
    lua["Speed"] = lua.create_table_with(
        "Normal", 0, 0, "Normal",
        "Slow", 1, 1, "Slow",
        "Fast", 2, 2, "Fast",
        "Faster", 3, 3, "Faster",
        "Fastest", 4, 4, "Fastest");
}

static LuaPoint getObjectPosition(GameObject* object) {
    return {object->getPositionX(), object->getPositionY() - 90.0f};
}
//updateObjectPosition with EditorUI is still required after this for the changes to work properly
static void setObjectPosition(GameObject* object, LuaPoint pos) {
    object->setPosition({static_cast<float>(pos.x), static_cast<float>(pos.y + 90.0f)});
}
void luaAddUsertypes(sol::state_view& lua, EditorUI* self) {
    auto pointType = lua.new_usertype<LuaPoint>("point", sol::constructors<LuaPoint(double, double)>(),
        "x", &LuaPoint::x,
        "y", &LuaPoint::y,
        "dot", &LuaPoint::dot,
        "magnitude", sol::readonly_property(&LuaPoint::magnitude),
        "unit", &LuaPoint::unit,
        sol::meta_function::addition, &LuaPoint::operator+,
        sol::meta_function::subtraction, static_cast<LuaPoint (LuaPoint::*)(LuaPoint) const>(&LuaPoint::operator-),
        sol::meta_function::multiplication, sol::overload(
            static_cast<LuaPoint (LuaPoint::*)(LuaPoint) const>(&LuaPoint::operator*),
            static_cast<LuaPoint (LuaPoint::*)(double) const>(&LuaPoint::operator*),
            static_cast<LuaPoint (*)(double, LuaPoint)>(&operator*)),
        sol::meta_function::division, sol::overload(
            static_cast<LuaPoint (LuaPoint::*)(LuaPoint) const>(&LuaPoint::operator/),
            static_cast<LuaPoint (LuaPoint::*)(double) const>(&LuaPoint::operator/),
            static_cast<LuaPoint (*)(double, LuaPoint)>(&operator/)),
        sol::meta_function::unary_minus, static_cast<LuaPoint (LuaPoint::*)() const>(&LuaPoint::operator-),
        sol::meta_function::to_string, &LuaPoint::operator std::string);
    setupImmutableMT(lua, lua["point"], pointType);
    lua["P"] = lua["point"]["new"]; //just make a shortcut since creating points is so common

    auto colorType = lua.new_usertype<ccColor3B>("color", sol::constructors<ccColor3B(GLubyte, GLubyte, GLubyte)>(),
        "r", &ccColor3B::r,
        "g", &ccColor3B::g,
        "b", &ccColor3B::b);
    setupImmutableMT(lua, lua["color"], colorType);
    auto hsvType = lua.new_usertype<ccHSVValue>("hsv", sol::constructors<ccHSVValue(float, float, float, bool, bool)>(),
        "h", &ccHSVValue::h,
        "s", &ccHSVValue::s,
        "v", &ccHSVValue::v,
        "absoluteSaturation", &ccHSVValue::absoluteSaturation,
        "absoluteBrightness", &ccHSVValue::absoluteBrightness);
    setupImmutableMT(lua, lua["hsv"], hsvType);

    auto gameObjectType = lua.new_usertype<GameObject>("GameObject", sol::no_constructor);
    setupImmutableMT(lua, lua["GameObject"], gameObjectType);
    gameObjectType["id"] = sol::readonly(&GameObject::m_objectID);
    gameObjectType["x"] = sol::property([](GameObject* object) {
        return object->getPositionX();
    }, [](GameObject* object, float x, curengine engine) {
        //object->m_positionX = x;
        object->setPositionX(x);
        updateObjectPosition(engine->editor, object);
    });
    gameObjectType["y"] = sol::property([](GameObject* object) {
        return object->getPositionY() - 90.0f;
    }, [](GameObject* object, float y, curengine engine) {
        //object->m_positionY = y + 90.0f;
        object->setPositionY(y + 90.0f);
        updateObjectPosition(engine->editor, object);
    });
    gameObjectType["pos"] = sol::property([](GameObject* object) {
        return getObjectPosition(object);
    }, [](GameObject* object, LuaPoint pos, curengine engine) {
        //object->m_positionX = pos.x;
        //object->m_positionY = pos.y + 90.0f;
        setObjectPosition(object, pos);
        updateObjectPosition(engine->editor, object);
    });
    gameObjectType["rotX"] = sol::property(&GameObject::getRotationX, &GameObject::setRotationX);
    gameObjectType["rotY"] = sol::property(&GameObject::getRotationY, &GameObject::setRotationY);
    gameObjectType["rot"] = sol::property(&GameObject::getRotation, &GameObject::setRotation);
    gameObjectType["scaleX"] = sol::property([](GameObject* object) {
        return object->m_scaleX;
    }, [](GameObject* object, float x) {
        //object->m_scaleX = x;
        //object->setScaleX(x * (object->m_isFlipX ? -1.0f : 1.0f));
        object->updateCustomScaleX(x);
    });
    gameObjectType["scaleY"] = sol::property([](GameObject* object) {
        return object->m_scaleY;
    }, [](GameObject* object, float y) {
        //object->m_scaleY = y;
        //object->setScaleX(y * (object->m_isFlipY ? -1.0f : 1.0f));
        object->updateCustomScaleY(y);
    });
    constexpr double PI = std::numbers::pi;
    constexpr double HALF_PI = PI * 0.5;
    constexpr double RADIANS_OVER_DEGREES = PI / 180.0;
    gameObjectType["vectorX"] = sol::property([](GameObject* object) {
        auto angle = -object->getRotationY() * RADIANS_OVER_DEGREES;
        return LuaPoint {std::cos(angle), std::sin(angle)} * (object->m_scaleX * 0.5);
    }, [](GameObject* object, LuaPoint vector) {
        object->setRotationY(-(std::atan2(vector.y, vector.x) / RADIANS_OVER_DEGREES));
        object->updateCustomScaleX(vector.magnitude() * 2.0);
    });
    gameObjectType["vectorY"] = sol::property([](GameObject* object) {
        double rot = object->getRotationX();
        //special case for 0 because for some reason cos(pi/2) is not exactly 0
        //i hate floating pointsssssssssss
        if (rot == 0.0) return LuaPoint {0, object->m_scaleY * 0.5};

        auto angle = HALF_PI - rot * RADIANS_OVER_DEGREES;
        return LuaPoint {std::cos(angle), std::sin(angle)} * (object->m_scaleY * 0.5);
    }, [](GameObject* object, LuaPoint vector) {
        object->setRotationX((HALF_PI - std::atan2(vector.y, vector.x)) / RADIANS_OVER_DEGREES);
        object->updateCustomScaleY(vector.magnitude() * 2.0);
    });
    gameObjectType["baseColorID"] = sol::property([](GameObject* object, sol::this_state lua) {
        auto color = object->m_baseColor;
        return color ? sol::make_object(lua, color->m_colorID) : sol::nil;
    }, [](GameObject* object, int id) {
        if (auto color = object->m_baseColor) {
            color->m_colorID = id;
            object->m_shouldUpdateColorSprite = true;
        }
    });
    gameObjectType["detailColorID"] = sol::property([](GameObject* object, sol::this_state lua) {
        auto color = object->m_detailColor;
        return color ? sol::make_object(lua, color->m_colorID) : sol::nil;
    }, [](GameObject* object, int id) {
        if (auto color = object->m_detailColor) {
            color->m_colorID = id;
            object->m_shouldUpdateColorSprite = true;
        }
    });
    gameObjectType["baseColorHSV"] = sol::property([](GameObject* object, sol::this_state lua) {
        auto color = object->m_baseColor;
        return color ? sol::make_object(lua, color->m_hsv) : sol::nil;
    }, [](GameObject* object, ccHSVValue hsv) {
        if (auto color = object->m_baseColor) {
            color->m_hsv = hsv;
            object->m_shouldUpdateColorSprite = true;
        }
    });
    gameObjectType["detailColorHSV"] = sol::property([](GameObject* object, sol::this_state lua) {
        auto color = object->m_detailColor;
        return color ? sol::make_object(lua, color->m_hsv) : sol::nil;
    }, [](GameObject* object, ccHSVValue hsv) {
        if (auto color = object->m_detailColor) {
            color->m_hsv = hsv;
            object->m_shouldUpdateColorSprite = true;
        }
    });
    log::debug("added GameObject properties 1");

    gameObjectType.set_function("getGroups", [](GameObject* object, sol::this_state lua) {
        auto output = sol::state_view(lua).create_table(object->m_groupCount);
        if (!object->m_groups) return output;
        for (int i = 0; i < object->m_groupCount; i++) {
            output[i] = object->m_groups->at(i);
        }
        return output;
    });
    gameObjectType.set_function("getParentGroups", [](GameObject* object, sol::this_state lua) {
        auto engine = LuaEngine::get(lua);
        auto output = sol::state_view(lua).create_table(object->m_groupCount);
        CCArrayExt<CCInteger*> parentGroups =
            static_cast<CCArray*>(engine->editor->m_editorLayer->m_parentGroupIDs->objectForKey(object->m_uniqueID));
        for (int i = 0; i < parentGroups.size(); i++) {
            output[i] = parentGroups[i]->getValue();
        }
        return output;
    });
    gameObjectType.set_function("addGroups", [](GameObject* object, sol::variadic_args args) {
        for (sol::optional<int> group : args) {
            if (!group.has_value()) continue;
            object->addToGroup(group.value());
        }
    });
    gameObjectType.set_function("addParentGroups", [](GameObject* object, sol::variadic_args args, curengine engine) {
        for (sol::optional<int> group : args) {
            if (!group.has_value()) continue;
            engine->editor->m_editorLayer->setGroupParent(object, group.value());
        }
    });
    gameObjectType.set_function("removeGroups", [](GameObject* object, sol::variadic_args args) {
        for (sol::optional<int> group : args) {
            if (!group.has_value()) continue;
            object->removeFromGroup(group.value());
        }
    });
    gameObjectType.set_function("resetGroups", &GameObject::resetGroups);

    gameObjectType.set_function("scaleAround", [](GameObject* object, LuaPoint origin, double scale, curengine engine) {
        LuaPoint offset = getObjectPosition(object) - origin;
        setObjectPosition(object, (offset * scale) + origin);
        updateObjectPosition(engine->editor, object);

        object->updateCustomScaleX(object->m_scaleX * scale);
        object->updateCustomScaleY(object->m_scaleY * scale);
    });
    gameObjectType.set_function("rotateAround", [](GameObject* object, LuaPoint origin, double rotation, curengine engine) {
        LuaPoint offset = getObjectPosition(object) - origin;
        double stdRotation = -rotation * RADIANS_OVER_DEGREES;
        double rotCos = std::cos(stdRotation), rotSin = std::sin(stdRotation);
        LuaPoint rotOffset = {
            offset.x * rotCos - offset.y * rotSin,
            offset.x * rotSin + offset.y * rotCos
        };
        setObjectPosition(object, rotOffset + origin);
        object->setRotationX(object->getRotationX() + rotation);
        object->setRotationY(object->getRotationY() + rotation);
    });
    gameObjectType.set_function("setPosition3P", [](GameObject* object, LuaPoint origin,
            LuaPoint pointX, LuaPoint pointY, sol::optional<int> flipMaskOpt, curengine engine) {
        int flipMask = flipMaskOpt.value_or(0);
        bool flipH = (flipMask & 1) == 1;
        bool flipV = (flipMask & 2) == 2;

        LuaPoint vectorX = (pointX - origin);
        LuaPoint vectorY = (pointY - origin);
        LuaPoint center = origin + (vectorX + vectorY) * 0.5;

        if (flipH) vectorX = -vectorX;
        if (flipV) vectorY = -vectorY;

        setObjectPosition(object, center);
        updateObjectPosition(engine->editor, object);

        object->setRotationY(-(std::atan2(vectorX.y, vectorX.x) / RADIANS_OVER_DEGREES));
        object->updateCustomScaleX(vectorX.magnitude() / 30.0);
        object->setRotationX((HALF_PI - std::atan2(vectorY.y, vectorY.x)) / RADIANS_OVER_DEGREES);
        object->updateCustomScaleY(vectorY.magnitude() / 30.0);
    });

    gameObjectType["linkedGroup"] = &GameObject::m_linkedGroup;

    addPropertiesForGameObjects(lua, self, gameObjectType);
    addPropertiesForTriggers(lua, self, gameObjectType);
}