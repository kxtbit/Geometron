#include "properties.hpp"

void addPropertiesForGameObjects(sol::state_view& lua, EditorUI* self, sol::usertype<GameObject> gameObjectType) {
#define BASE_PROPERTY(name) gameObjectType[#name] = &GameObject::m_##name
#define BASE_RENAME_PROPERTY(newName, oldName) gameObjectType[#newName] = &GameObject::m_##oldName
#define BASE_READONLY_PROPERTY(name) gameObjectType[#name] = sol::readonly(&GameObject::m_##name)
#define BASE_READONLY_RENAME_PROPERTY(newName, oldName) gameObjectType[#newName] = sol::readonly(&GameObject::m_##oldName)
#define SUBCLASS_PROPERTY(type, name) gameObjectType[#name] = gameObjectSubclassProperty<&type::m_##name>()
#define SUBCLASS_READONLY_PROPERTY(type, name) gameObjectType[#name] = gameObjectSubclassReadOnlyProperty<&type::m_##name>()
#define SUBCLASS_READONLY_POINT_PROPERTY(type, name) gameObjectType[#name] = gameObjectSubclassReadOnlyPointProperty<&type::m_##name>()
#define SUBCLASS_HOOKED_PROPERTY(type, name, hook) gameObjectType[#name] = gameObjectSubclassProperty<&type::m_##name, hook>()

    { //other object properties
        gameObjectType["isFlipX"] = sol::property(&GameObject::isFlipX, &GameObject::setFlipX);
        gameObjectType["isFlipY"] = sol::property(&GameObject::isFlipY, &GameObject::setFlipY);
        BASE_RENAME_PROPERTY(singleColorType, customColorType);
        BASE_PROPERTY(mainColorKeyIndex);
        BASE_PROPERTY(detailColorKeyIndex);
        BASE_PROPERTY(editorLayer);
        BASE_PROPERTY(editorLayer2);
        gameObjectType["zLayer"] = sol::property(propertyGetter<&GameObject::m_zLayer>, [](GameObject* object, ZLayer layer) {
            if (!object->m_zFixedZLayer) {
                object->m_zLayer = layer;
                //object->m_shouldUpdateColorSprite = true;
                //i guess this value is called that now? idk
                object->m_updateParents = true;
            }
        });
        BASE_READONLY_RENAME_PROPERTY(fixedZLayer, zFixedZLayer);
        gameObjectType["zOrder"] = sol::property(propertyGetter<&GameObject::m_zOrder>, [](GameObject* object, int order) {
            object->m_zOrder = order;
            //object->m_shouldUpdateColorSprite = true;
            object->m_updateParents = true;
        });
        BASE_READONLY_PROPERTY(objectType);
        BASE_READONLY_PROPERTY(classType);
        BASE_READONLY_PROPERTY(isDecoration);
        BASE_READONLY_PROPERTY(isDecoration2);
    }
    { //extra object properties
        BASE_PROPERTY(isDontEnter);
        BASE_PROPERTY(isDontFade);
        BASE_RENAME_PROPERTY(isNoEffects, hasNoEffects);
        BASE_RENAME_PROPERTY(isGroupParent, hasGroupParent);
        BASE_RENAME_PROPERTY(isAreaParent, hasAreaParent);
        BASE_PROPERTY(isDontBoostX);
        BASE_PROPERTY(isDontBoostY);
        BASE_PROPERTY(isHighDetail);
        gameObjectType["isNoTouch"] = sol::property(propertyGetter<&GameObject::m_isNoTouch>, [](GameObject* object, bool noTouch, curengine engine) {
            bool exists = gameObjectExists(object);

            auto editorLayer = engine->editor->m_editorLayer;
            if (object->m_isNoTouch == noTouch) return;

            if (exists)
                editorLayer->removeObjectFromSection(object);
            object->m_isNoTouch = noTouch;
            object->setType(object->m_savedObjectType);
            object->saveActiveColors(); //saveActiveColors also updates m_objectType to Decoration if notouch
            if (exists)
                editorLayer->addToSection(object);
        });
        BASE_PROPERTY(isPassable);
        BASE_PROPERTY(isHide);
        BASE_PROPERTY(isNonStickX);
        BASE_PROPERTY(isNonStickY);
        BASE_PROPERTY(isExtraSticky);
        BASE_PROPERTY(isScaleStick);
        gameObjectType["isExtendedCollision"] = sol::property(propertyGetter<&GameObject::m_hasExtendedCollision>, [](GameObject* object, bool noTouch, curengine engine) {
            bool exists = gameObjectExists(object);

            auto editorLayer = engine->editor->m_editorLayer;
            if (object->m_hasExtendedCollision == noTouch) return;

            if (exists)
                editorLayer->removeObjectFromSection(object);
            object->m_hasExtendedCollision = noTouch;
            if (exists)
                editorLayer->addToSection(object);
        });
        BASE_PROPERTY(isIceBlock);
        BASE_PROPERTY(isGripSlope);
        BASE_RENAME_PROPERTY(isNoGlow, hasNoGlow);
        BASE_RENAME_PROPERTY(isNoParticles, hasNoParticles);
        BASE_RENAME_PROPERTY(isNoAudioScale, hasNoAudioScale);
        BASE_PROPERTY(enterChannel);
    }
    { //EnhancedGameObject
        WriteHook(EnhancedGameObject, rotatingObjectHook, {
            object->createRotateAction(object->m_rotationAngle, 0);
        });
        WriteHook(EnhancedGameObject, animatingObjectHook, {
            object->setupAnimationVariables();
        });

        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, poweredOn);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, state);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, animationRandomizedStartValue);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, animationStart);
        //gameObjectType["unk540"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_unk540);
        //gameObjectType["unk544"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_unk544);
        //gameObjectType["unk548"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_unk548);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, randomFrameTime);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, visible);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, shouldNotHideAnimFreeze);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, usesSpecialAnimation);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, frameTime);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, frames);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, hasCustomAnimation);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, hasCustomRotation);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, disableRotation, rotatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, rotationSpeed, rotatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, rotationAngle, rotatingObjectHook::f);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, rotationDelta);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, rotationAnimationSpeed);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, animationRandomizedStart, animatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, animationSpeed, animatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, animationShouldUseSpeed, animatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, animateOnTrigger, animatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, disableDelayedLoop, animatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, disableAnimShine, animatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, singleFrame, animatingObjectHook::f);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, animationOffset, animatingObjectHook::f);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, animationTriggered);
        //gameObjectType["unkAnimationInt"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_unkAnimationInt);
        //gameObjectType["maybeAnimationVariableXInt"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_maybeAnimationVariableXInt);
        //gameObjectType["maybeAnimationVariableYInt"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_maybeAnimationVariableYInt);
        SUBCLASS_HOOKED_PROPERTY(EnhancedGameObject, animateOnlyWhenActive, animatingObjectHook::f);
        SUBCLASS_PROPERTY(EnhancedGameObject, isNoMultiActivate);
        SUBCLASS_PROPERTY(EnhancedGameObject, isMultiActivate);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, activated);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, activatedByPlayer1);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, activatedByPlayer2);
        SUBCLASS_READONLY_PROPERTY(EnhancedGameObject, hasUniqueCoin);
    }

    { //orb parameters
        SUBCLASS_PROPERTY(RingObject, claimTouch);
        SUBCLASS_PROPERTY(RingObject, isSpawnOnly);

        SUBCLASS_PROPERTY(DashRingObject, dashSpeed);
        SUBCLASS_PROPERTY(DashRingObject, endBoost);
        SUBCLASS_PROPERTY(DashRingObject, maxDuration);
        SUBCLASS_PROPERTY(DashRingObject, allowCollide);
        SUBCLASS_PROPERTY(DashRingObject, stopSlide);
    }

    { //teleport portals
        gameObjectType["orangePortal"] = sol::property([](GameObject* object, sol::this_state lua) {
            return wrapGameObject(lua, subclassCast<TeleportPortalObject>(object, lua)->m_orangePortal);
        }, [](GameObject* object, GameObject* value, sol::this_state lua) {
            subclassCast<TeleportPortalObject>(object, lua)->m_orangePortal =
                subclassCast<TeleportPortalObject>(value, lua);
        });
        SUBCLASS_READONLY_PROPERTY(TeleportPortalObject, isYellowPortal);
        SUBCLASS_PROPERTY(TeleportPortalObject, teleportYOffset);
        SUBCLASS_PROPERTY(TeleportPortalObject, teleportEase);
        SUBCLASS_PROPERTY(TeleportPortalObject, staticForceEnabled);
        SUBCLASS_PROPERTY(TeleportPortalObject, staticForce);
        SUBCLASS_PROPERTY(TeleportPortalObject, redirectForceEnabled);
        SUBCLASS_PROPERTY(TeleportPortalObject, redirectForceMod);
        SUBCLASS_PROPERTY(TeleportPortalObject, redirectForceMin);
        SUBCLASS_PROPERTY(TeleportPortalObject, redirectForceMax);
        SUBCLASS_PROPERTY(TeleportPortalObject, saveOffset);
        SUBCLASS_PROPERTY(TeleportPortalObject, ignoreX);
        SUBCLASS_PROPERTY(TeleportPortalObject, ignoreY);
        SUBCLASS_PROPERTY(TeleportPortalObject, gravityMode);
        SUBCLASS_PROPERTY(TeleportPortalObject, staticForceAdditive);
        SUBCLASS_PROPERTY(TeleportPortalObject, instantCamera);
        SUBCLASS_PROPERTY(TeleportPortalObject, snapGround);
        SUBCLASS_PROPERTY(TeleportPortalObject, redirectDash);
        SUBCLASS_READONLY_POINT_PROPERTY(TeleportPortalObject, teleportPosition);
    }

    gameObjectType["text"] = sol::property([](GameObject* object, sol::this_state lua) {
        return std::string(subclassCast<TextGameObject>(object, lua)->m_text);
    }, [](GameObject* object, std::string str, sol::this_state lua) {
        checkObjectExists(lua, object);
        subclassCast<TextGameObject>(object, lua)->updateTextObject(str, false);
    });
    gameObjectType["kerning"] = sol::property([](GameObject* object, sol::this_state lua) {
        return subclassCast<TextGameObject>(object, lua)->m_kerning;
    }, [](GameObject* object, int kerning, sol::this_state lua) {
        checkObjectExists(lua, object);
        auto text = subclassCast<TextGameObject>(object, lua);
        text->updateTextKerning(kerning);
        text->updateTextObject(text->m_text, false);
    });
    gameObjectType.set_function("splitText", [](GameObject* object, sol::optional<bool> deleteOriginal, curengine engine, sol::this_state lua) {
        checkObjectExists(lua, object);
        auto objs = splitText(engine->editor, subclassCast<TextGameObject>(object, lua));
        if (deleteOriginal.value_or(true)) safeDeleteObject(engine->editor, object);
        return wrapArrayOfGameObjects(lua, objs.inner());
    });
}