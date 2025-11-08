// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppDeclarationHidesUncapturedLocal
#include <numbers>

#include <Geode/Geode.hpp>

#include "LuaEngine.hpp"
#include "LuaUtils.hpp"
#include "bindings/properties.hpp"
#include "../utils/types/GPoint.hpp"
#include "../utils/types/GTransform.hpp"

using namespace geode::prelude;

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
    lua["SingleColorType"] = lua.create_table_with(
        "Default", 0, 0, "Default",
        "Base", 1, 1, "Base",
        "Detail", 2, 2, "Detail");
}

static GPoint getObjectPosition(GameObject* object) {
    return {object->getPositionX(), object->getPositionY() - 90.0f};
}
//updateObjectPosition with EditorUI is still required after this for the changes to work properly
static void setObjectPosition(GameObject* object, GPoint pos) {
    object->setPosition({static_cast<float>(pos.x), static_cast<float>(pos.y + 90.0f)});
}
void luaAddUsertypes(sol::state_view& lua, EditorUI* self) {
    auto pointType = lua.new_usertype<GPoint>("point", sol::constructors<GPoint(double, double)>(),
        "x", &GPoint::x,
        "y", &GPoint::y,
        "dot", &GPoint::dot,
        "magnitude", sol::readonly_property(&GPoint::magnitude),
        "unit", &GPoint::unit,
        sol::meta_function::addition, &GPoint::operator+,
        sol::meta_function::subtraction, static_cast<GPoint (GPoint::*)(const GPoint&) const>(&GPoint::operator-),
        sol::meta_function::multiplication, sol::overload(
            static_cast<GPoint (GPoint::*)(const GPoint&) const>(&GPoint::operator*),
            static_cast<GPoint (GPoint::*)(double) const>(&GPoint::operator*),
            static_cast<GPoint (*)(double, GPoint)>(&operator*)),
        sol::meta_function::division, sol::overload(
            static_cast<GPoint (GPoint::*)(const GPoint&) const>(&GPoint::operator/),
            static_cast<GPoint (GPoint::*)(double) const>(&GPoint::operator/),
            static_cast<GPoint (*)(double, GPoint)>(&operator/)),
        sol::meta_function::unary_minus, static_cast<GPoint (GPoint::*)() const>(&GPoint::operator-),
        sol::meta_function::to_string, &GPoint::operator std::string);
    setupImmutableMT(lua, lua["point"], pointType);

    auto transformType = lua.new_usertype<GTransform>("transform", sol::constructors<GTransform(double, double, double, double, double, double)>(),
        "xx", &GTransform::xx, "xy", &GTransform::xy, "xc", &GTransform::xc,
        "yx", &GTransform::yx, "yy", &GTransform::yy, "yc", &GTransform::yc,
        "identity", sol::var(GTransform::identity()),
        //static functions
        "translation", sol::overload(
            static_cast<GTransform (*)(double, double)>(&GTransform::translation),
            static_cast<GTransform (*)(const GPoint&)>(&GTransform::translation)),
        "scaling", sol::overload(
            static_cast<GTransform (*)(double, double)>(&GTransform::scaling),
            static_cast<GTransform (*)(const GPoint&)>(&GTransform::scaling)),
        "shear", &GTransform::shear,
        "rotation", &GTransform::rotation,
        "dualRotation", &GTransform::dualRotation,
        "fromVectors", &GTransform::fromVectors,
        //properties
        "pos", sol::readonly_property(&GTransform::pos),
        "scale", sol::readonly_property(&GTransform::scale),
        "rotXY", sol::readonly_property(&GTransform::rotXY),
        "degXY", sol::readonly_property(&GTransform::degXY),
        "inverse", sol::readonly_property(&GTransform::inverse),
        //methods
        "apply", sol::overload(
            static_cast<GPoint (GTransform::*)(const GPoint&) const>(&GTransform::apply),
            static_cast<GTransform (GTransform::*)(const GTransform&) const>(&GTransform::apply)),
        "applyVector", &GTransform::applyVector,
        "chain", &GTransform::chain,
        //metatable operations
        sol::meta_function::addition, &GTransform::operator+,
        sol::meta_function::to_string, &GTransform::operator std::string);
    setupImmutableMT(lua, lua["transform"], transformType);

    auto colorType = lua.new_usertype<ccColor3B>("color", sol::constructors<ccColor3B(GLubyte, GLubyte, GLubyte)>(),
        "r", sol::readonly_property(&ccColor3B::r),
        "g", sol::readonly_property(&ccColor3B::g),
        "b", sol::readonly_property(&ccColor3B::b),
        sol::meta_function::to_string, [](ccColor3B color) {return fmt::format("color {{{}, {}, {}}}", color.r, color.g, color.b);});
    setupImmutableMT(lua, lua["color"], colorType);
    auto hsvType = lua.new_usertype<ccHSVValue>("hsv", sol::constructors<ccHSVValue(float, float, float, bool, bool)>(),
        "h", sol::readonly_property(&ccHSVValue::h),
        "s", sol::readonly_property(&ccHSVValue::s),
        "v", sol::readonly_property(&ccHSVValue::v),
        "addS", sol::readonly_property(&ccHSVValue::absoluteSaturation),
        "addV", sol::readonly_property(&ccHSVValue::absoluteBrightness),
        sol::meta_function::to_string, [](ccHSVValue hsv) {
            return fmt::format("hsv {{{}, {}{}, {}{}}}",
                hsv.h,
                hsv.absoluteSaturation ? "+" : "*", hsv.s,
                hsv.absoluteBrightness ? "+" : "*", hsv.v);
        });
    setupImmutableMT(lua, lua["hsv"], hsvType);

    auto gameObjectType = lua.new_usertype<GameObject>("GameObject", sol::no_constructor,
        sol::meta_function::equal_to, [](GameObject* l, GameObject* r) {
            return l->m_uniqueID == r->m_uniqueID;
        });
    setupImmutableMT(lua, lua["GameObject"], gameObjectType);
    gameObjectType["id"] = sol::readonly(&GameObject::m_objectID);
    gameObjectType["uid"] = sol::readonly(&GameObject::m_uniqueID);
    gameObjectType["linkID"] = sol::readonly(&GameObject::m_linkedGroup);
    gameObjectType["pos"] = sol::property([](GameObject* object) {
        return getObjectPosition(object);
    }, [](GameObject* object, GPoint pos, curengine engine) {
        //object->m_positionX = pos.x;
        //object->m_positionY = pos.y + 90.0f;
        setObjectPosition(object, pos);
        updateObjectPosition(engine->editor, object);
    });
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
    gameObjectType["rotX"] = sol::property(&GameObject::getRotationY, &GameObject::setRotationY);
    gameObjectType["rotY"] = sol::property(&GameObject::getRotationX, &GameObject::setRotationX);
    gameObjectType["rot"] = sol::property(&GameObject::getRotation, &GameObject::setRotation);
    gameObjectType["scale"] = sol::property([](GameObject* object) {
        return std::max(object->m_scaleX, object->m_scaleY);
    }, [](GameObject* object, float scale) {
        object->updateCustomScaleX(scale);
        object->updateCustomScaleY(scale);
    });
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
        return GPoint {std::cos(angle), std::sin(angle)} * (object->m_scaleX * 0.5);
    }, [](GameObject* object, GPoint vector) {
        object->setRotationY(-(std::atan2(vector.y, vector.x) / RADIANS_OVER_DEGREES));
        object->updateCustomScaleX(vector.magnitude() * 2.0);
    });
    gameObjectType["vectorY"] = sol::property([](GameObject* object) {
        double rot = object->getRotationX();
        //special case for 0 because for some reason cos(pi/2) is not exactly 0
        //i hate floating pointsssssssssss
        if (rot == 0.0) return GPoint {0, object->m_scaleY * 0.5};

        auto angle = HALF_PI - rot * RADIANS_OVER_DEGREES;
        return GPoint {std::cos(angle), std::sin(angle)} * (object->m_scaleY * 0.5);
    }, [](GameObject* object, GPoint vector) {
        object->setRotationX((HALF_PI - std::atan2(vector.y, vector.x)) / RADIANS_OVER_DEGREES);
        object->updateCustomScaleY(vector.magnitude() * 2.0);
    });
    gameObjectType["transform"] = sol::property([](GameObject* object) {
        return GTransform::ofObject(object);
    }, [](GameObject* object, const GTransform& transform, curengine engine) {
        auto pos = transform.pos();
        auto scale = transform.scale();
        auto rot = -transform.degXY();

        setObjectPosition(object, pos);
        updateObjectPosition(engine->editor, object);
        object->setRotationY(rot.x);
        object->setRotationX(rot.y);
        object->updateCustomScaleX(scale.x);
        object->updateCustomScaleY(scale.y);
    });
    gameObjectType["baseColorID"] = sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
        auto color = object->m_baseColor;
        if (!color) return sol::nil;
        auto id = color->m_colorID;
        if (id == 0) id = color->m_defaultColorID;
        return sol::make_object(lua, id);
    }, [](GameObject* object, int id) {
        if (id < 0) id = 0;
        if (auto color = object->m_baseColor) {
            color->m_colorID = id;
            object->m_shouldUpdateColorSprite = true;
        }
    });
    gameObjectType["detailColorID"] = sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
        auto color = object->m_detailColor;
        if (!color) return sol::nil;
        auto id = color->m_colorID;
        if (id == 0) id = color->m_defaultColorID;
        return sol::make_object(lua, id);
    }, [](GameObject* object, int id) {
        if (id < 0) id = 0;
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

    gameObjectType["isSelected"] = sol::readonly_property(&GameObject::m_isSelected);
    gameObjectType["exists"] = sol::readonly_property([](GameObject* object) {
        return gameObjectExists(object);
    });

    gameObjectType.set_function("getGroups", [](GameObject* object, sol::this_state lua) {
        auto output = sol::state_view(lua).create_table(object->m_groupCount);
        if (!object->m_groups) return output;
        for (int i = 0; i < object->m_groupCount; i++) {
            output[i + 1] = object->m_groups->at(i);
        }
        return output;
    });
    gameObjectType.set_function("addGroup", [](GameObject* object, int group, curengine engine, sol::this_state lua) {
        checkObjectExists(lua, object);

        if (object->addToGroup(group) != 0) {
            engine->editor->m_editorLayer->addToGroup(object, group, false);
            return true;
        }
        return false;
    });
    gameObjectType.set_function("addGroups", [](GameObject* object, sol::lua_table groups, curengine engine, sol::this_state lua) {
        checkObjectExists(lua, object);

        bool success = true;
        auto editorLayer = engine->editor->m_editorLayer;
        for (int i = 1; i <= groups.size(); i++) {
            sol::optional<int> group = groups[i];
            if (!group.has_value()) continue;
            //addToGroup result of 0 means there was no room to add the group
            if (object->addToGroup(group.value()) != 0)
                editorLayer->addToGroup(object, group.value(), false);
            else success = false;
        }

        return success;
    });
    gameObjectType.set_function("removeGroup", [](GameObject* object, int group, curengine engine, sol::this_state lua) {
        checkObjectExists(lua, object);

        if (object->belongsToGroup(group)) {
            object->removeFromGroup(group);
            engine->editor->m_editorLayer->removeFromGroup(object, group);
            return true;
        }
        return false;
    });
    gameObjectType.set_function("removeGroups", [](GameObject* object, sol::lua_table groups, curengine engine, sol::this_state lua) {
        checkObjectExists(lua, object);

        bool success = true;
        auto editorLayer = engine->editor->m_editorLayer;
        for (int i = 1; i <= groups.size(); i++) {
            sol::optional<int> group = groups[i];
            if (object->belongsToGroup(group.value())) {
                object->removeFromGroup(group.value());
                editorLayer->removeFromGroup(object, group.value());
            } else success = false;
        }

        return success;
    });
    gameObjectType.set_function("resetGroups", [](GameObject* object, curengine engine, sol::this_state lua) {
        checkObjectExists(lua, object);

        engine->editor->m_editorLayer->removeFromGroups(object);
        object->resetGroups();
    });
    gameObjectType.set_function("hasGroup", [](GameObject* object, int group) {
        return object->belongsToGroup(group);
    });

    gameObjectType.set_function("scaleAround", [](GameObject* object, GPoint origin, double scale, curengine engine) {
        GPoint offset = getObjectPosition(object) - origin;
        setObjectPosition(object, (offset * scale) + origin);
        updateObjectPosition(engine->editor, object);

        object->updateCustomScaleX(object->m_scaleX * scale);
        object->updateCustomScaleY(object->m_scaleY * scale);
    });
    gameObjectType.set_function("rotateAround", [](GameObject* object, GPoint origin, double rotation, curengine engine) {
        GPoint offset = getObjectPosition(object) - origin;
        double stdRotation = -rotation * RADIANS_OVER_DEGREES;
        double rotCos = std::cos(stdRotation), rotSin = std::sin(stdRotation);
        GPoint rotOffset = {
            offset.x * rotCos - offset.y * rotSin,
            offset.x * rotSin + offset.y * rotCos
        };
        setObjectPosition(object, rotOffset + origin);
        object->setRotationX(object->getRotationX() + rotation);
        object->setRotationY(object->getRotationY() + rotation);
    });
    gameObjectType.set_function("setPosition3P", [](GameObject* object, GPoint origin,
            GPoint pointX, GPoint pointY, sol::optional<int> flipMaskOpt, curengine engine) {
        int flipMask = flipMaskOpt.value_or(0);
        bool flipH = (flipMask & 1) == 1;
        bool flipV = (flipMask & 2) == 2;

        GPoint vectorX = (pointX - origin);
        GPoint vectorY = (pointY - origin);
        GPoint center = origin + (vectorX + vectorY) * 0.5;

        if (flipH) vectorX = -vectorX;
        if (flipV) vectorY = -vectorY;

        setObjectPosition(object, center);
        updateObjectPosition(engine->editor, object);

        object->setRotationY(-(std::atan2(vectorX.y, vectorX.x) / RADIANS_OVER_DEGREES));
        object->updateCustomScaleX(vectorX.magnitude() / 30.0);
        object->setRotationX((HALF_PI - std::atan2(vectorY.y, vectorY.x)) / RADIANS_OVER_DEGREES);
        object->updateCustomScaleY(vectorY.magnitude() / 30.0);
    });

    addPropertiesForGameObjects(lua, self, gameObjectType);
    addPropertiesForTriggers(lua, self, gameObjectType);
}