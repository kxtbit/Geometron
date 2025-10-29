#include "properties.hpp"

void addPropertiesForGameObjects(sol::state_view& lua, EditorUI* self, sol::usertype<GameObject> gameObjectType) {
    { //other object properties
        gameObjectType["isFlipX"] = sol::property(&GameObject::isFlipX, &GameObject::setFlipX);
        gameObjectType["isFlipY"] = sol::property(&GameObject::isFlipY, &GameObject::setFlipY);
        gameObjectType["customColorType"] = &GameObject::m_customColorType;
        gameObjectType["mainColorKeyIndex"] = &GameObject::m_mainColorKeyIndex;
        gameObjectType["detailColorKeyIndex"] = &GameObject::m_detailColorKeyIndex;
        gameObjectType["editorLayer"] = &GameObject::m_editorLayer;
        gameObjectType["editorLayer2"] = &GameObject::m_editorLayer2;
        gameObjectType["zLayer"] = sol::property([](GameObject* object) {
            return object->m_zLayer;
        }, [](GameObject* object, ZLayer layer) {
            if (!object->m_zFixedZLayer) {
                object->m_zLayer = layer;
                object->m_shouldUpdateColorSprite = true;
            }
        });
        gameObjectType["zFixedZLayer"] = sol::readonly(&GameObject::m_zFixedZLayer);
        gameObjectType["zOrder"] = sol::property([](GameObject* object) {
            return object->m_zOrder;
        }, [](GameObject* object, int order) {
            object->m_zOrder = order;
            object->m_shouldUpdateColorSprite = true;
        });
        gameObjectType["classType"] = sol::readonly(&GameObject::m_classType);
        log::debug("added GameObject properties 3");
    }
    { //extra object properties
        gameObjectType["isDontEnter"] = &GameObject::m_isDontEnter;
        gameObjectType["isDontFade"] = &GameObject::m_isDontFade;
        gameObjectType["hasNoEffects"] = &GameObject::m_hasNoEffects;
        gameObjectType["hasGroupParent"] = &GameObject::m_hasGroupParent;
        gameObjectType["hasAreaParent"] = &GameObject::m_hasAreaParent;
        gameObjectType["isDontBoostX"] = &GameObject::m_isDontBoostX;
        gameObjectType["isDontBoostY"] = &GameObject::m_isDontBoostY;
        gameObjectType["isHighDetail"] = &GameObject::m_isHighDetail;
        gameObjectType["isNoTouch"] = sol::property(&GameObject::m_isNoTouch, [](GameObject* object, bool noTouch, curengine engine) {
            auto editorLayer = engine->editor->m_editorLayer;
            if (object->m_isNoTouch == noTouch) return;

            editorLayer->removeObjectFromSection(object);
            object->m_isNoTouch = noTouch;
            object->setType(object->m_savedObjectType);
            object->saveActiveColors(); //saveActiveColors also updates m_objectType to Decoration if notouch
            editorLayer->addToSection(object);
        });
        gameObjectType["isPassable"] = &GameObject::m_isPassable;
        gameObjectType["isHide"] = &GameObject::m_isHide;
        gameObjectType["isNonStickX"] = &GameObject::m_isNonStickX;
        gameObjectType["isNonStickY"] = &GameObject::m_isNonStickY;
        gameObjectType["isExtraSticky"] = &GameObject::m_isExtraSticky;
        gameObjectType["hasExtendedCollision"] = sol::property(&GameObject::m_hasExtendedCollision, [](GameObject* object, bool noTouch, curengine engine) {
            auto editorLayer = engine->editor->m_editorLayer;
            if (object->m_hasExtendedCollision == noTouch) return;

            editorLayer->removeObjectFromSection(object);
            object->m_hasExtendedCollision = noTouch;
            editorLayer->addToSection(object);
        });
        gameObjectType["isIceBlock"] = &GameObject::m_isIceBlock;
        gameObjectType["isGripSlope"] = &GameObject::m_isGripSlope;
        gameObjectType["hasNoGlow"] = &GameObject::m_hasNoGlow;
        gameObjectType["hasNoParticles"] = &GameObject::m_hasNoParticles;
        gameObjectType["hasNoAudioScale"] = &GameObject::m_hasNoAudioScale;
        gameObjectType["enterChannel"] = &GameObject::m_enterChannel;
        log::debug("added GameObject properties 4");
    }
    { //EnhancedGameObject
        WriteHook(EnhancedGameObject, rotatingObjectHook, {
            object->createRotateAction(object->m_rotationAngle, 0);
        });
        WriteHook(EnhancedGameObject, animatingObjectHook, {
            object->setupAnimationVariables();
        });

        gameObjectType["poweredOn"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_poweredOn>();
        gameObjectType["state"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_state>();
        gameObjectType["animationRandomizedStartValue"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_animationRandomizedStartValue>();
        gameObjectType["animationStart"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_animationStart>();
        //gameObjectType["unk540"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_unk540);
        //gameObjectType["unk544"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_unk544);
        //gameObjectType["unk548"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_unk548);
        gameObjectType["randomFrameTime"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_randomFrameTime>();
        gameObjectType["visible"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_visible>();
        gameObjectType["shouldNotHideAnimFreeze"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_shouldNotHideAnimFreeze>();
        gameObjectType["usesSpecialAnimation"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_usesSpecialAnimation>();
        gameObjectType["frameTime"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_frameTime>();
        gameObjectType["frames"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_frames>();
        gameObjectType["hasCustomAnimation"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_hasCustomAnimation>();
        gameObjectType["hasCustomRotation"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_hasCustomRotation>();
        gameObjectType["disableRotation"] = gameObjectSubclassProperty<&EnhancedGameObject::m_disableRotation, rotatingObjectHook::f>();
        gameObjectType["rotationSpeed"] = gameObjectSubclassProperty<&EnhancedGameObject::m_rotationSpeed, rotatingObjectHook::f>();
        gameObjectType["rotationAngle"] = gameObjectSubclassProperty<&EnhancedGameObject::m_rotationAngle, rotatingObjectHook::f>();
        gameObjectType["rotationDelta"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_rotationDelta>();
        gameObjectType["rotationAnimationSpeed"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_rotationAnimationSpeed>();
        gameObjectType["animationRandomizedStart"] = gameObjectSubclassProperty<&EnhancedGameObject::m_animationRandomizedStart, animatingObjectHook::f>();
        gameObjectType["animationSpeed"] = gameObjectSubclassProperty<&EnhancedGameObject::m_animationSpeed, animatingObjectHook::f>();
        gameObjectType["animationShouldUseSpeed"] = gameObjectSubclassProperty<&EnhancedGameObject::m_animationShouldUseSpeed, animatingObjectHook::f>();
        gameObjectType["animateOnTrigger"] = gameObjectSubclassProperty<&EnhancedGameObject::m_animateOnTrigger, animatingObjectHook::f>();
        gameObjectType["disableDelayedLoop"] = gameObjectSubclassProperty<&EnhancedGameObject::m_disableDelayedLoop, animatingObjectHook::f>();
        gameObjectType["disableAnimShine"] = gameObjectSubclassProperty<&EnhancedGameObject::m_disableAnimShine, animatingObjectHook::f>();
        gameObjectType["singleFrame"] = gameObjectSubclassProperty<&EnhancedGameObject::m_singleFrame, animatingObjectHook::f>();
        gameObjectType["animationOffset"] = gameObjectSubclassProperty<&EnhancedGameObject::m_animationOffset, animatingObjectHook::f>();
        gameObjectType["animationTriggered"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_animationTriggered>();
        //gameObjectType["unkAnimationInt"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_unkAnimationInt);
        //gameObjectType["maybeAnimationVariableXInt"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_maybeAnimationVariableXInt);
        //gameObjectType["maybeAnimationVariableYInt"] = gameObjectSubclassProperty(lua, &EnhancedGameObject::m_maybeAnimationVariableYInt);
        gameObjectType["animateOnlyWhenActive"] = gameObjectSubclassProperty<&EnhancedGameObject::m_animateOnlyWhenActive, animatingObjectHook::f>();
        gameObjectType["isNoMultiActivate"] = gameObjectSubclassProperty<&EnhancedGameObject::m_isNoMultiActivate>();
        gameObjectType["isMultiActivate"] = gameObjectSubclassProperty<&EnhancedGameObject::m_isMultiActivate>();
        gameObjectType["activated"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_activated>();
        gameObjectType["activatedByPlayer1"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_activatedByPlayer1>();
        gameObjectType["activatedByPlayer2"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_activatedByPlayer2>();
        gameObjectType["hasUniqueCoin"] = gameObjectSubclassReadOnlyProperty<&EnhancedGameObject::m_hasUniqueCoin>();
        log::debug("added GameObject properties 5");
    }

    { //orb parameters
        gameObjectType["claimTouch"] = gameObjectSubclassProperty<&RingObject::m_claimTouch>();
        gameObjectType["isSpawnOnly"] = gameObjectSubclassProperty<&RingObject::m_isSpawnOnly>();

        gameObjectType["dashSpeed"] = gameObjectSubclassProperty<&DashRingObject::m_dashSpeed>();
        gameObjectType["endBoost"] = gameObjectSubclassProperty<&DashRingObject::m_endBoost>();
        gameObjectType["maxDuration"] = gameObjectSubclassProperty<&DashRingObject::m_maxDuration>();
        gameObjectType["allowCollide"] = gameObjectSubclassProperty<&DashRingObject::m_allowCollide>();
        gameObjectType["stopSlide"] = gameObjectSubclassProperty<&DashRingObject::m_stopSlide>();
        log::debug("added GameObject properties 6");
    }

    WriteHook(TextGameObject, textHook, {
        object->updateTextObject(object->m_text, false);
    });

    gameObjectType["text"] = gameObjectSubclassProperty<&TextGameObject::m_text, textHook::f>();
    gameObjectType["kerning"] = gameObjectSubclassProperty<&TextGameObject::m_kerning>();
}