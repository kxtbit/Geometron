#include "properties.hpp"

void addPropertiesForTriggers(sol::state_view& lua, EditorUI* self, sol::usertype<GameObject> gameObjectType) {
    { //trigger parameters
        //used to signal the editor to update the label for a trigger when it should change
        WriteHook(GameObject, labelHook, {
            LuaEngine::get(lua)->editor->m_editorLayer->updateObjectLabel(object);
        });
        WriteHook(EffectGameObject, durationHook, {
            //i THINK the global variable the base game uses here is supposed to be a preallocated point {0, 0}
            if (!object->m_dontIgnoreDuration)
                object->m_endPosition = CCPoint {0, 0};
        });

        gameObjectType["targetColorID"] = sol::property(&GameObject::m_targetColor, [](GameObject* object, int id, sol::this_state lua) {
            object->m_targetColor = id;
            labelHook::f(object, lua);
        }); //this one is on GameObject for some reason, no idea why

        gameObjectType["triggerTargetColor"] = gameObjectSubclassProperty<&EffectGameObject::m_triggerTargetColor>();
        gameObjectType["duration"] = gameObjectSubclassProperty<&EffectGameObject::m_duration, durationHook::f>();
        gameObjectType["opacity"] = gameObjectSubclassProperty<&EffectGameObject::m_opacity>();
        gameObjectType["triggerEffectPlaying"] = gameObjectSubclassProperty<&EffectGameObject::m_triggerEffectPlaying>();
        gameObjectType["targetGroupID"] = gameObjectSubclassProperty<&EffectGameObject::m_targetGroupID, labelHook::f>();
        gameObjectType["centerGroupID"] = gameObjectSubclassProperty<&EffectGameObject::m_centerGroupID, labelHook::f>();
        gameObjectType["isTouchTriggered"] = gameObjectSubclassProperty<&EffectGameObject::m_isTouchTriggered>();
        gameObjectType["isSpawnTriggered"] = gameObjectSubclassProperty<&EffectGameObject::m_isSpawnTriggered>();
        gameObjectType["hasCenterEffect"] = gameObjectSubclassProperty<&EffectGameObject::m_hasCenterEffect>();
        gameObjectType["shakeStrength"] = gameObjectSubclassProperty<&EffectGameObject::m_shakeStrength>();
        gameObjectType["shakeInterval"] = gameObjectSubclassProperty<&EffectGameObject::m_shakeInterval>();
        gameObjectType["tintGround"] = gameObjectSubclassProperty<&EffectGameObject::m_tintGround>();
        gameObjectType["usesPlayerColor1"] = gameObjectSubclassProperty<&EffectGameObject::m_usesPlayerColor1>();
        gameObjectType["usesPlayerColor2"] = gameObjectSubclassProperty<&EffectGameObject::m_usesPlayerColor2>();
        gameObjectType["usesBlending"] = gameObjectSubclassProperty<&EffectGameObject::m_usesBlending>();
        gameObjectType["moveOffset"] = gameObjectSubclassPointProperty<&EffectGameObject::m_moveOffset>();
        gameObjectType["easingType"] = gameObjectSubclassProperty<&EffectGameObject::m_easingType>();
        gameObjectType["easingRate"] = gameObjectSubclassProperty<&EffectGameObject::m_easingRate>();
        gameObjectType["lockToPlayerX"] = gameObjectSubclassProperty<&EffectGameObject::m_lockToPlayerX>();
        gameObjectType["lockToPlayerY"] = gameObjectSubclassProperty<&EffectGameObject::m_lockToPlayerY>();
        gameObjectType["lockToCameraX"] = gameObjectSubclassProperty<&EffectGameObject::m_lockToCameraX>();
        gameObjectType["lockToCameraY"] = gameObjectSubclassProperty<&EffectGameObject::m_lockToCameraY>();
        gameObjectType["useMoveTarget"] = gameObjectSubclassProperty<&EffectGameObject::m_useMoveTarget>();
        gameObjectType["moveTargetMode"] = gameObjectSubclassProperty<&EffectGameObject::m_moveTargetMode>();
        gameObjectType["moveModX"] = gameObjectSubclassProperty<&EffectGameObject::m_moveModX>();
        gameObjectType["moveModY"] = gameObjectSubclassProperty<&EffectGameObject::m_moveModY>();
        gameObjectType["smallStep"] = gameObjectSubclassProperty<&EffectGameObject::m_smallStep>();
        gameObjectType["isDirectionFollowSnap360"] = gameObjectSubclassProperty<&EffectGameObject::m_isDirectionFollowSnap360>();
        gameObjectType["targetModCenterID"] = gameObjectSubclassProperty<&EffectGameObject::m_targetModCenterID>();
        gameObjectType["directionModeDistance"] = gameObjectSubclassProperty<&EffectGameObject::m_directionModeDistance>();
        gameObjectType["isDynamicMode"] = gameObjectSubclassProperty<&EffectGameObject::m_isDynamicMode>();
        gameObjectType["isSilent"] = gameObjectSubclassProperty<&EffectGameObject::m_isSilent>();
        gameObjectType["specialTarget"] = gameObjectSubclassProperty<&EffectGameObject::m_specialTarget>();
        gameObjectType["rotationDegrees"] = gameObjectSubclassProperty<&EffectGameObject::m_rotationDegrees>();
        gameObjectType["times360"] = gameObjectSubclassProperty<&EffectGameObject::m_times360>();
        gameObjectType["lockObjectRotation"] = gameObjectSubclassProperty<&EffectGameObject::m_lockObjectRotation>();
        gameObjectType["rotationTargetID"] = gameObjectSubclassProperty<&EffectGameObject::m_rotationTargetID>();
        gameObjectType["rotationOffset"] = gameObjectSubclassProperty<&EffectGameObject::m_rotationOffset>();
        gameObjectType["dynamicModeEasing"] = gameObjectSubclassProperty<&EffectGameObject::m_dynamicModeEasing>();
        gameObjectType["followXMod"] = gameObjectSubclassProperty<&EffectGameObject::m_followXMod>();
        gameObjectType["followYMod"] = gameObjectSubclassProperty<&EffectGameObject::m_followYMod>();
        gameObjectType["followYSpeed"] = gameObjectSubclassProperty<&EffectGameObject::m_followYSpeed>();
        gameObjectType["followYDelay"] = gameObjectSubclassProperty<&EffectGameObject::m_followYDelay>();
        gameObjectType["followYOffset"] = gameObjectSubclassProperty<&EffectGameObject::m_followYOffset>();
        gameObjectType["followYMaxSpeed"] = gameObjectSubclassProperty<&EffectGameObject::m_followYMaxSpeed>();
        gameObjectType["fadeInDuration"] = gameObjectSubclassProperty<&EffectGameObject::m_fadeInDuration, durationHook::f>();
        gameObjectType["holdDuration"] = gameObjectSubclassProperty<&EffectGameObject::m_holdDuration, durationHook::f>();
        gameObjectType["fadeOutDuration"] = gameObjectSubclassProperty<&EffectGameObject::m_fadeOutDuration, durationHook::f>();
        gameObjectType["pulseMode"] = gameObjectSubclassProperty<&EffectGameObject::m_pulseMode>();
        gameObjectType["pulseTargetType"] = gameObjectSubclassProperty<&EffectGameObject::m_pulseTargetType, labelHook::f>();
        gameObjectType["hsvValue"] = gameObjectSubclassProperty<&EffectGameObject::m_hsvValue>();
        gameObjectType["copyColorID"] = gameObjectSubclassProperty<&EffectGameObject::m_copyColorID>();
        gameObjectType["copyOpacity"] = gameObjectSubclassProperty<&EffectGameObject::m_copyOpacity>();
        gameObjectType["pulseMainOnly"] = gameObjectSubclassProperty<&EffectGameObject::m_pulseMainOnly>();
        gameObjectType["pulseDetailOnly"] = gameObjectSubclassProperty<&EffectGameObject::m_pulseDetailOnly>();
        gameObjectType["pulseExclusive"] = gameObjectSubclassProperty<&EffectGameObject::m_pulseExclusive>();
        gameObjectType["legacyHSV"] = gameObjectSubclassProperty<&EffectGameObject::m_legacyHSV>();
        gameObjectType["activateGroup"] = gameObjectSubclassProperty<&EffectGameObject::m_activateGroup>();
        gameObjectType["touchHoldMode"] = gameObjectSubclassProperty<&EffectGameObject::m_touchHoldMode>();
        gameObjectType["touchToggleMode"] = gameObjectSubclassProperty<&EffectGameObject::m_touchToggleMode>();
        gameObjectType["touchPlayerMode"] = gameObjectSubclassProperty<&EffectGameObject::m_touchPlayerMode>();
        gameObjectType["isDualMode"] = gameObjectSubclassProperty<&EffectGameObject::m_isDualMode>();
        gameObjectType["animationID"] = gameObjectSubclassProperty<&EffectGameObject::m_animationID>();
        gameObjectType["spawnXPosition"] = gameObjectSubclassProperty<&EffectGameObject::m_spawnXPosition>();
        gameObjectType["spawnOrder"] = gameObjectSubclassProperty<&EffectGameObject::m_spawnOrder>();
        gameObjectType["isMultiTriggered"] = gameObjectSubclassProperty<&EffectGameObject::m_isMultiTriggered>();
        gameObjectType["previewDisable"] = gameObjectSubclassProperty<&EffectGameObject::m_previewDisable>();
        gameObjectType["spawnOrdered"] = gameObjectSubclassProperty<&EffectGameObject::m_spawnOrdered>();
        gameObjectType["triggerOnExit"] = gameObjectSubclassProperty<&EffectGameObject::m_triggerOnExit>();
        gameObjectType["itemID2"] = gameObjectSubclassProperty<&EffectGameObject::m_itemID2>();
        gameObjectType["controlID"] = gameObjectSubclassProperty<&EffectGameObject::m_controlID>();
        gameObjectType["targetControlID"] = gameObjectSubclassProperty<&EffectGameObject::m_targetControlID>();
        gameObjectType["isDynamicBlock"] = gameObjectSubclassProperty<&EffectGameObject::m_isDynamicBlock, labelHook::f>();
        gameObjectType["itemID"] = gameObjectSubclassProperty<&EffectGameObject::m_itemID, labelHook::f>();
        gameObjectType["targetPlayer1"] = gameObjectSubclassProperty<&EffectGameObject::m_targetPlayer1>();
        gameObjectType["targetPlayer2"] = gameObjectSubclassProperty<&EffectGameObject::m_targetPlayer2>();
        gameObjectType["followCPP"] = gameObjectSubclassProperty<&EffectGameObject::m_followCPP>();
        gameObjectType["subtractCount"] = gameObjectSubclassProperty<&EffectGameObject::m_subtractCount>();
        gameObjectType["collectibleIsPickupItem"] = gameObjectSubclassProperty<&EffectGameObject::m_collectibleIsPickupItem>();
        gameObjectType["collectibleIsToggleTrigger"] = gameObjectSubclassProperty<&EffectGameObject::m_collectibleIsToggleTrigger>();
        gameObjectType["collectibleParticleID"] = gameObjectSubclassProperty<&EffectGameObject::m_collectibleParticleID>();
        gameObjectType["collectiblePoints"] = gameObjectSubclassProperty<&EffectGameObject::m_collectiblePoints>();
        gameObjectType["hasNoAnimation"] = gameObjectSubclassProperty<&EffectGameObject::m_hasNoAnimation>();
        gameObjectType["forceModID"] = gameObjectSubclassProperty<&EffectGameObject::m_forceModID>();
        gameObjectType["rotateFollowP1"] = gameObjectSubclassProperty<&EffectGameObject::m_rotateFollowP1>();
        gameObjectType["rotateFollowP2"] = gameObjectSubclassProperty<&EffectGameObject::m_rotateFollowP2>();
        gameObjectType["gravityValue"] = gameObjectSubclassProperty<&EffectGameObject::m_gravityValue>();
        gameObjectType["isSinglePTouch"] = gameObjectSubclassProperty<&EffectGameObject::m_isSinglePTouch>();
        gameObjectType["zoomValue"] = gameObjectSubclassProperty<&EffectGameObject::m_zoomValue>();
        gameObjectType["cameraIsFreeMode"] = gameObjectSubclassProperty<&EffectGameObject::m_cameraIsFreeMode>();
        gameObjectType["cameraEditCameraSettings"] = gameObjectSubclassProperty<&EffectGameObject::m_cameraEditCameraSettings>();
        gameObjectType["cameraEasingValue"] = gameObjectSubclassProperty<&EffectGameObject::m_cameraEasingValue>();
        gameObjectType["cameraPaddingValue"] = gameObjectSubclassProperty<&EffectGameObject::m_cameraPaddingValue>();
        gameObjectType["cameraDisableGridSnap"] = gameObjectSubclassProperty<&EffectGameObject::m_cameraDisableGridSnap>();
        gameObjectType["endReversed"] = gameObjectSubclassProperty<&EffectGameObject::m_endReversed>();
        gameObjectType["timeWarpTimeMod"] = gameObjectSubclassProperty<&EffectGameObject::m_timeWarpTimeMod>();
        gameObjectType["shouldPreview"] = sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
            if (auto effect = typeinfo_cast<EffectGameObject*>(object)) {
                return make_object(lua, effect->m_shouldPreview);
            }
            return sol::nil;
        }, [](GameObject* object, bool shouldPreview, curengine engine) {
            if (auto effect = typeinfo_cast<EffectGameObject*>(object)) {
                effect->m_shouldPreview = shouldPreview;
                engine->editor->m_editorLayer->tryUpdateSpeedObject(effect, false);
            }
        });
        gameObjectType["ordValue"] = gameObjectSubclassProperty<&EffectGameObject::m_ordValue>();
        gameObjectType["channelValue"] = gameObjectSubclassProperty<&EffectGameObject::m_channelValue>();
        gameObjectType["isReverse"] = gameObjectSubclassProperty<&EffectGameObject::m_isReverse>();
        gameObjectType["speedModType"] = gameObjectSubclassReadOnlyProperty<&EffectGameObject::m_speedModType>();
        gameObjectType["speedStart"] = gameObjectSubclassReadOnlyPointProperty<&EffectGameObject::m_speedStart>();
        gameObjectType["secretCoinID"] = gameObjectSubclassReadOnlyProperty<&EffectGameObject::m_secretCoinID>();
        gameObjectType["endPosition"] = gameObjectSubclassReadOnlyPointProperty<&EffectGameObject::m_endPosition>();
        gameObjectType["spawnTriggerDelay"] = gameObjectSubclassProperty<&EffectGameObject::m_spawnTriggerDelay>();
        gameObjectType["gravityMod"] = gameObjectSubclassProperty<&EffectGameObject::m_gravityMod>();
        gameObjectType["ignoreGroupParent"] = gameObjectSubclassProperty<&EffectGameObject::m_ignoreGroupParent>();
        gameObjectType["ignoreLinkedObjects"] = gameObjectSubclassProperty<&EffectGameObject::m_ignoreLinkedObjects>();
        gameObjectType["channelChanged"] = gameObjectSubclassReadOnlyProperty<&EffectGameObject::m_channelChanged>();

        gameObjectType["isSinglePTouch"] = gameObjectSubclassProperty<&EffectGameObject::m_isSinglePTouch>();
        gameObjectType["hasCenterEffect"] = gameObjectSubclassProperty<&EffectGameObject::m_hasCenterEffect>();
        gameObjectType["isReverse"] = gameObjectSubclassProperty<&EffectGameObject::m_isReverse>();
        gameObjectType["controlID"] = gameObjectSubclassProperty<&EffectGameObject::m_controlID>();


        gameObjectType["exitStatic"] = gameObjectSubclassProperty<&CameraTriggerGameObject::m_exitStatic>();
        gameObjectType["followObject"] = gameObjectSubclassProperty<&CameraTriggerGameObject::m_followObject>();
        gameObjectType["followEasing"] = gameObjectSubclassProperty<&CameraTriggerGameObject::m_followEasing>();
        gameObjectType["edgeDirection"] = gameObjectSubclassProperty<&CameraTriggerGameObject::m_edgeDirection>();
        gameObjectType["smoothVelocity"] = gameObjectSubclassProperty<&CameraTriggerGameObject::m_smoothVelocity>();
        gameObjectType["velocityModifier"] = gameObjectSubclassProperty<&CameraTriggerGameObject::m_velocityModifier>();
        gameObjectType["exitInstant"] = gameObjectSubclassProperty<&CameraTriggerGameObject::m_exitInstant>();
        gameObjectType["previewOpacity"] = gameObjectSubclassProperty<&CameraTriggerGameObject::m_previewOpacity>();

        gameObjectType["pickupCount"] = gameObjectSubclassProperty<&CountTriggerGameObject::m_pickupCount>();
        gameObjectType["pickupTriggerMode"] = gameObjectSubclassProperty<&CountTriggerGameObject::m_pickupTriggerMode>();
        gameObjectType["multiActivate"] = gameObjectSubclassProperty<&CountTriggerGameObject::m_multiActivate>();
        gameObjectType["isOverride"] = gameObjectSubclassProperty<&CountTriggerGameObject::m_isOverride>();
        gameObjectType["pickupTriggerMultiplier"] = gameObjectSubclassProperty<&CountTriggerGameObject::m_pickupTriggerMultiplier>();

        //why so many properties for advanced follow trigger aaaa
        {
            gameObjectType["delay"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_delay>();
            gameObjectType["delayVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_delayVariance>();
            gameObjectType["startSpeed"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_startSpeed>();
            gameObjectType["startSpeedVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_startSpeedVariance>();
            gameObjectType["startSpeedReference"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_startSpeedReference>();
            gameObjectType["startDirection"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_startDirection>();
            gameObjectType["startDirectionVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_startDirectionVariance>();
            gameObjectType["startDirectionReference"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_startDirectionReference>();
            gameObjectType["maxSpeed"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_maxSpeed>();
            gameObjectType["maxSpeedVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_maxSpeedVariance>();
            gameObjectType["xOnly"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_xOnly>();
            gameObjectType["yOnly"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_yOnly>();
            gameObjectType["maxRange"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_maxRange>();
            gameObjectType["maxRangeVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_maxRangeVariance>();
            //gameObjectType["property310"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property310>();
            //gameObjectType["property311"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property311>();
            gameObjectType["acceleration"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_acceleration>();
            gameObjectType["accelerationVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_accelerationVariance>();
            //gameObjectType["property312"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property312>();
            //gameObjectType["property313"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property313>();
            //gameObjectType["property314"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property314>();
            //gameObjectType["property315"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property315>();
            gameObjectType["steerForce"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_steerForce>();
            gameObjectType["steerForceVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_steerForceVariance>();
            gameObjectType["steerForceLowEnabled"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_steerForceLowEnabled>();
            gameObjectType["steerForceLow"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_steerForceLow>();
            gameObjectType["steerForceLowVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_steerForceLowVariance>();
            gameObjectType["steerForceHighEnabled"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_steerForceHighEnabled>();
            gameObjectType["steerForceHigh"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_steerForceHigh>();
            gameObjectType["steerFroceHighVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_steerFroceHighVariance>();
            gameObjectType["speedRangeLow"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_speedRangeLow>();
            gameObjectType["speedRangeLowVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_speedRangeLowVariance>();
            gameObjectType["speedRangeHigh"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_speedRangeHigh>();
            gameObjectType["speedRangeHighVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_speedRangeHighVariance>();
            gameObjectType["breakForce"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_breakForce>();
            gameObjectType["breakForceVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_breakForceVariance>();
            gameObjectType["breakAngle"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_breakAngle>();
            gameObjectType["breakAngleVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_breakAngleVariance>();
            gameObjectType["breakSteerForce"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_breakSteerForce>();
            gameObjectType["breakSteerForceVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_breakSteerForceVariance>();
            gameObjectType["breakSteerSpeedLimit"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_breakSteerSpeedLimit>();
            gameObjectType["breakSteerSpeedLimitVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_breakSteerSpeedLimitVariance>();
            gameObjectType["targetDirection"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_targetDirection>();
            gameObjectType["ignoreDisabled"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_ignoreDisabled>();
            gameObjectType["rotateDirection"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_rotateDirection>();
            gameObjectType["rotationOffset"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_rotationOffset>();
            gameObjectType["nearAcceleration"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_nearAcceleration>();
            gameObjectType["nearAccelerationVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_nearAccelerationVariance>();
            gameObjectType["nearDistance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_nearDistance>();
            gameObjectType["nearDistanceVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_nearDistanceVariance>();
            gameObjectType["nearFriction"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_nearFriction>();
            gameObjectType["nearFrictionVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_nearFrictionVariance>();
            gameObjectType["friction"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_friction>();
            gameObjectType["frictionVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_frictionVariance>();
            gameObjectType["easing"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_easing>();
            gameObjectType["easingVariance"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_easingVariance>();
            gameObjectType["rotateEasing"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_rotateEasing>();
            gameObjectType["rotateDeadZ"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_rotateDeadZ>();
            gameObjectType["priority"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_priority>();
            //gameObjectType["unk7fc"] = gameObjectSubclassProperty(lua, &AdvancedFollowTriggerObject::m_unk7fc);
            gameObjectType["maxRangeReference"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_maxRangeReference>();
            gameObjectType["followMode"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_followMode>();
            gameObjectType["exclusive"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_exclusive>();
            gameObjectType["startMode"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_startMode>();
        }

        gameObjectType["modX"] = gameObjectSubclassProperty<&AdvancedFollowEditObject::m_modX>();
        gameObjectType["modXVariance"] = gameObjectSubclassProperty<&AdvancedFollowEditObject::m_modXVariance>();
        gameObjectType["modY"] = gameObjectSubclassProperty<&AdvancedFollowEditObject::m_modY>();
        gameObjectType["modYVariance"] = gameObjectSubclassProperty<&AdvancedFollowEditObject::m_modYVariance>();
        gameObjectType["redirectDirection"] = gameObjectSubclassProperty<&AdvancedFollowEditObject::m_redirectDirection>();

        gameObjectType["noEffects"] = gameObjectSubclassProperty<&EndTriggerGameObject::m_noEffects>();
        gameObjectType["noSFX"] = gameObjectSubclassProperty<&EndTriggerGameObject::m_noSFX>();
        gameObjectType["instant"] = gameObjectSubclassProperty<&EndTriggerGameObject::m_instant>();

        gameObjectType["enterType"] = gameObjectSubclassProperty<&EnterEffectObject::m_enterType>();
        gameObjectType["length"] = gameObjectSubclassProperty<&EnterEffectObject::m_length>();
        gameObjectType["lengthVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_lengthVariance>();
        gameObjectType["offset"] = gameObjectSubclassProperty<&EnterEffectObject::m_offset>();
        gameObjectType["offsetVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_offsetVariance>();
        gameObjectType["offsetY"] = gameObjectSubclassProperty<&EnterEffectObject::m_offsetY>();
        gameObjectType["offsetYVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_offsetYVariance>();
        gameObjectType["moveDistance"] = gameObjectSubclassProperty<&EnterEffectObject::m_moveDistance>();
        gameObjectType["moveDistanceVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_moveDistanceVariance>();
        gameObjectType["areaScaleX"] = gameObjectSubclassProperty<&EnterEffectObject::m_areaScaleX>();
        gameObjectType["areaScaleXVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_areaScaleXVariance>();
        gameObjectType["areaScaleY"] = gameObjectSubclassProperty<&EnterEffectObject::m_areaScaleY>();
        gameObjectType["areaScaleYVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_areaScaleYVariance>();
        gameObjectType["moveAngle"] = gameObjectSubclassProperty<&EnterEffectObject::m_moveAngle>();
        gameObjectType["moveAngleVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_moveAngleVariance>();
        gameObjectType["startAngle"] = gameObjectSubclassProperty<&EnterEffectObject::m_startAngle>();
        gameObjectType["anglePosition"] = gameObjectSubclassProperty<&EnterEffectObject::m_anglePosition>();
        gameObjectType["relative"] = gameObjectSubclassProperty<&EnterEffectObject::m_relative>();
        gameObjectType["relativeFade"] = gameObjectSubclassProperty<&EnterEffectObject::m_relativeFade>();
        gameObjectType["easingInType"] = gameObjectSubclassProperty<&EnterEffectObject::m_easingInType>();
        gameObjectType["easingInRate"] = gameObjectSubclassProperty<&EnterEffectObject::m_easingInRate>();
        gameObjectType["easingInBuffer"] = gameObjectSubclassProperty<&EnterEffectObject::m_easingInBuffer>();
        gameObjectType["easingOutType"] = gameObjectSubclassProperty<&EnterEffectObject::m_easingOutType>();
        gameObjectType["easingOutRate"] = gameObjectSubclassProperty<&EnterEffectObject::m_easingOutRate>();
        gameObjectType["easingOutBuffer"] = gameObjectSubclassProperty<&EnterEffectObject::m_easingOutBuffer>();
        gameObjectType["moveX"] = gameObjectSubclassProperty<&EnterEffectObject::m_moveX>();
        gameObjectType["moveXVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_moveXVariance>();
        gameObjectType["moveY"] = gameObjectSubclassProperty<&EnterEffectObject::m_moveY>();
        gameObjectType["moveYVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_moveYVariance>();
        gameObjectType["tintChannelID"] = gameObjectSubclassProperty<&EnterEffectObject::m_tintChannelID>();
        //gameObjectType["property224"] = gameObjectSubclassProperty<&EnterEffectObject::m_property224>();
        gameObjectType["directionType"] = gameObjectSubclassProperty<&EnterEffectObject::m_directionType>();
        gameObjectType["xyMode"] = gameObjectSubclassProperty<&EnterEffectObject::m_xyMode>();
        gameObjectType["easeOutEnabled"] = gameObjectSubclassProperty<&EnterEffectObject::m_easeOutEnabled>();
        gameObjectType["modFront"] = gameObjectSubclassProperty<&EnterEffectObject::m_modFront>();
        gameObjectType["modBack"] = gameObjectSubclassProperty<&EnterEffectObject::m_modBack>();
        gameObjectType["areaTint"] = gameObjectSubclassProperty<&EnterEffectObject::m_areaTint>();
        //gameObjectType["property285"] = gameObjectSubclassProperty<&EnterEffectObject::m_property285>();
        gameObjectType["effectID"] = gameObjectSubclassProperty<&EnterEffectObject::m_effectID>();
        gameObjectType["areaRotation"] = gameObjectSubclassProperty<&EnterEffectObject::m_areaRotation>();
        gameObjectType["areaRotationVariance"] = gameObjectSubclassProperty<&EnterEffectObject::m_areaRotationVariance>();
        gameObjectType["toOpacity"] = gameObjectSubclassProperty<&EnterEffectObject::m_toOpacity>();
        gameObjectType["fromOpacity"] = gameObjectSubclassProperty<&EnterEffectObject::m_fromOpacity>();
        gameObjectType["inbound"] = gameObjectSubclassProperty<&EnterEffectObject::m_inbound>();
        gameObjectType["hsvEnabled"] = gameObjectSubclassProperty<&EnterEffectObject::m_hsvEnabled>();
        gameObjectType["deadzone"] = gameObjectSubclassProperty<&EnterEffectObject::m_deadzone>();
        gameObjectType["twoDirections"] = gameObjectSubclassProperty<&EnterEffectObject::m_twoDirections>();
        gameObjectType["dontEditAreaParent"] = gameObjectSubclassProperty<&EnterEffectObject::m_dontEditAreaParent>();
        gameObjectType["priority"] = gameObjectSubclassProperty<&EnterEffectObject::m_priority>();
        //gameObjectType["unk7d8"] = gameObjectSubclassProperty(lua, &EnterEffectObject::m_unk7d8);
        gameObjectType["enterChannel"] = gameObjectSubclassProperty<&EnterEffectObject::m_enterChannel>();
        gameObjectType["useEffectID"] = gameObjectSubclassProperty<&EnterEffectObject::m_useEffectID>();
        //gameObjectType["unk7e4"] = gameObjectSubclassProperty(lua, &EnterEffectObject::m_unk7e4);
        //gameObjectType["unk7ec"] = gameObjectSubclassProperty(lua, &EnterEffectObject::m_unk7ec);
        gameObjectType["negativeTargetX"] = gameObjectSubclassProperty<&EnterEffectObject::m_negativeTargetX>();
        gameObjectType["areaRange"] = gameObjectSubclassProperty<&EnterEffectObject::m_areaRange>();
        //gameObjectType["unk7fc"] = gameObjectSubclassProperty(lua, &EnterEffectObject::m_unk7fc);

        gameObjectType["resetRemap"] = gameObjectSubclassProperty<&EventLinkTrigger::m_resetRemap>();
        gameObjectType["extraID"] = gameObjectSubclassProperty<&EventLinkTrigger::m_extraID>();
        gameObjectType["extraID2"] = gameObjectSubclassProperty<&EventLinkTrigger::m_extraID2>();
        gameObjectType["eventIDCount"] = sol::readonly_property([&lua](GameObject* rawObject) -> sol::object {
            auto object = typeinfo_cast<EventLinkTrigger*>(rawObject);
            if (!object) return sol::nil;
            return sol::make_object(lua, static_cast<int>(object->m_eventIDs.size()));
        });
        gameObjectType.set_function("eventGetIDs", [&lua](GameObject* rawObject) -> sol::object {
            auto object = typeinfo_cast<EventLinkTrigger*>(rawObject);
            if (!object) return sol::nil;
            auto eventIDs = object->m_eventIDs;
            auto table = lua.create_table(eventIDs.size());
            int i = 1;
            for (int id : eventIDs) {
                table[i++] = id;
            }
            return table;
        });
        gameObjectType.set_function("eventAddIDs", [](GameObject* rawObject, sol::variadic_args ids) {
            auto object = typeinfo_cast<EventLinkTrigger*>(rawObject);
            if (!object) return;
            for (sol::optional<int> id : ids) {
                if (!id.has_value()) continue;
                object->m_eventIDs.erase(id.value());
            }
        });
        gameObjectType.set_function("eventSetIDs", [](GameObject* rawObject, sol::variadic_args ids) {
            auto object = typeinfo_cast<EventLinkTrigger*>(rawObject);
            if (!object) return;
            object->m_eventIDs.clear();
            for (sol::optional<int> id : ids) {
                if (!id.has_value()) continue;
                object->m_eventIDs.erase(id.value());
            }
        });
        gameObjectType.set_function("eventRemoveIDs", [](GameObject* rawObject, sol::variadic_args ids) {
            auto object = typeinfo_cast<EventLinkTrigger*>(rawObject);
            if (!object) return;
            for (sol::optional<int> id : ids) {
                if (!id.has_value()) continue;
                object->m_eventIDs.extract(id.value());
            }
        });

        gameObjectType["force"] = gameObjectSubclassProperty<&ForceBlockGameObject::m_force>();
        gameObjectType["minForce"] = gameObjectSubclassProperty<&ForceBlockGameObject::m_minForce>();
        gameObjectType["maxForce"] = gameObjectSubclassProperty<&ForceBlockGameObject::m_maxForce>();
        gameObjectType["relativeForce"] = gameObjectSubclassProperty<&ForceBlockGameObject::m_relativeForce>();
        gameObjectType["forceRange"] = gameObjectSubclassProperty<&ForceBlockGameObject::m_forceRange>();
        gameObjectType["forceID"] = gameObjectSubclassProperty<&ForceBlockGameObject::m_forceID>();

        gameObjectType["streakAdditive"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_streakAdditive>();
        gameObjectType["unlinkDualGravity"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_unlinkDualGravity>();
        gameObjectType["hideGround"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_hideGround>();
        gameObjectType["hideP1"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_hideP1>();
        gameObjectType["hideP2"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_hideP2>();
        gameObjectType["disableP1Controls"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_disableP1Controls>();
        gameObjectType["disableP2Controls"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_disableP2Controls>();
        gameObjectType["hideMG"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_hideMG>();
        gameObjectType["hideAttempts"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_hideAttempts>();
        gameObjectType["editRespawnTime"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_editRespawnTime>();
        gameObjectType["respawnTime"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_respawnTime>();
        gameObjectType["audioOnDeath"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_audioOnDeath>();
        gameObjectType["noDeathSFX"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_noDeathSFX>();
        gameObjectType["boostSlide"] = gameObjectSubclassProperty<&GameOptionsTrigger::m_boostSlide>();

        gameObjectType["blendingLayer"] = gameObjectSubclassProperty<&GradientTriggerObject::m_blendingLayer>();
        gameObjectType["blendingMode"] = gameObjectSubclassProperty<&GradientTriggerObject::m_blendingMode>();
        gameObjectType["gradientID"] = gameObjectSubclassProperty<&GradientTriggerObject::m_gradientID>();
        gameObjectType["upBottomLeftID"] = gameObjectSubclassProperty<&GradientTriggerObject::m_upBottomLeftID>();
        gameObjectType["downBottomRightID"] = gameObjectSubclassProperty<&GradientTriggerObject::m_downBottomRightID>();
        gameObjectType["leftTopLeftID"] = gameObjectSubclassProperty<&GradientTriggerObject::m_leftTopLeftID>();
        gameObjectType["rightTopRightID"] = gameObjectSubclassProperty<&GradientTriggerObject::m_rightTopRightID>();
        gameObjectType["vertexMode"] = gameObjectSubclassProperty<&GradientTriggerObject::m_vertexMode>();
        gameObjectType["disable"] = gameObjectSubclassProperty<&GradientTriggerObject::m_disable>();
        gameObjectType["disableAll"] = gameObjectSubclassProperty<&GradientTriggerObject::m_disableAll>();
        gameObjectType["previewOpacity"] = gameObjectSubclassProperty<&GradientTriggerObject::m_previewOpacity>();

        gameObjectType["item1Mode"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_item1Mode>();
        gameObjectType["item2Mode"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_item2Mode>();
        gameObjectType["targetItemMode"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_targetItemMode>();
        gameObjectType["mod1"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_mod1>();
        gameObjectType["mod2"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_mod2>();
        gameObjectType["resultType1"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_resultType1>();
        gameObjectType["resultType2"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_resultType2>();
        gameObjectType["resultType3"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_resultType3>();
        gameObjectType["tolerance"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_tolerance>();
        gameObjectType["roundType1"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_roundType1>();
        gameObjectType["roundType2"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_roundType2>();
        gameObjectType["signType1"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_signType1>();
        gameObjectType["signType2"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_signType2>();
        gameObjectType["persistent"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_persistent>();
        gameObjectType["targetAll"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_targetAll>();
        gameObjectType["reset"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_reset>();
        gameObjectType["timer"] = gameObjectSubclassProperty<&ItemTriggerGameObject::m_timer>();

        gameObjectType["timeMod"] = gameObjectSubclassProperty<&KeyframeAnimTriggerObject::m_timeMod>();
        gameObjectType["positionXMod"] = gameObjectSubclassProperty<&KeyframeAnimTriggerObject::m_positionXMod>();
        gameObjectType["positionYMod"] = gameObjectSubclassProperty<&KeyframeAnimTriggerObject::m_positionYMod>();
        gameObjectType["rotationMod"] = gameObjectSubclassProperty<&KeyframeAnimTriggerObject::m_rotationMod>();
        gameObjectType["scaleXMod"] = gameObjectSubclassProperty<&KeyframeAnimTriggerObject::m_scaleXMod>();
        gameObjectType["scaleYMod"] = gameObjectSubclassProperty<&KeyframeAnimTriggerObject::m_scaleYMod>();

        gameObjectType["keyframeGroup"] = gameObjectSubclassReadOnlyProperty<&KeyframeGameObject::m_keyframeGroup>();
        gameObjectType["keyframeIndex"] = gameObjectSubclassReadOnlyProperty<&KeyframeGameObject::m_keyframeIndex>();
        gameObjectType["referenceOnly"] = gameObjectSubclassProperty<&KeyframeGameObject::m_referenceOnly>();
        gameObjectType["proximity"] = gameObjectSubclassProperty<&KeyframeGameObject::m_proximity>();
        gameObjectType["curve"] = gameObjectSubclassProperty<&KeyframeGameObject::m_curve>();
        gameObjectType["closeLoop"] = gameObjectSubclassProperty<&KeyframeGameObject::m_closeLoop>();
        gameObjectType["timeMode"] = gameObjectSubclassProperty<&KeyframeGameObject::m_timeMode>();
        //gameObjectType["unk760"] = gameObjectSubclassProperty(lua, &KeyframeGameObject::m_unk760);
        gameObjectType["spawnDelay"] = gameObjectSubclassProperty<&KeyframeGameObject::m_spawnDelay>();
        gameObjectType["previewArt"] = gameObjectSubclassProperty<&KeyframeGameObject::m_previewArt>();
        gameObjectType["keyframeActive"] = gameObjectSubclassProperty<&KeyframeGameObject::m_keyframeActive>();
        gameObjectType["autoLayer"] = gameObjectSubclassProperty<&KeyframeGameObject::m_autoLayer>();
        gameObjectType["direction"] = gameObjectSubclassProperty<&KeyframeGameObject::m_direction>();
        gameObjectType["revolutions"] = gameObjectSubclassProperty<&KeyframeGameObject::m_revolutions>();
        gameObjectType["lineOpacity"] = gameObjectSubclassProperty<&KeyframeGameObject::m_lineOpacity>();

        gameObjectType["startTime"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_startTime>();
        gameObjectType["targetTime"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_targetTime>();
        gameObjectType["stopTimeEnabled"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_stopTimeEnabled>();
        gameObjectType["dontOverride"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_dontOverride>();
        gameObjectType["ignoreTimeWarp"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_ignoreTimeWarp>();
        gameObjectType["timeMod"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_timeMod>();
        gameObjectType["startPaused"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_startPaused>();
        gameObjectType["multiActivate"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_multiActivate>();
        gameObjectType["controlType"] = gameObjectSubclassProperty<&TimerTriggerGameObject::m_controlType>();

        gameObjectType["objectScaleX"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_objectScaleX>();
        gameObjectType["objectScaleY"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_objectScaleY>();
        //gameObjectType["property450"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_property450>();
        //gameObjectType["property451"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_property451>();
        gameObjectType["onlyMove"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_onlyMove>();
        gameObjectType["divideX"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_divideX>();
        gameObjectType["divideY"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_divideY>();
        gameObjectType["relativeRotation"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_relativeRotation>();
        gameObjectType["relativeScale"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_relativeScale>();

        gameObjectType["advRandTargetCount"] = sol::readonly_property([&lua](GameObject* rawObject) -> sol::object {
            auto object = typeinfo_cast<ChanceTriggerGameObject*>(rawObject);
            if (!object) return sol::nil;
            return sol::make_object(lua, static_cast<int>(object->m_chanceObjects.size()));
        });
        gameObjectType.set_function("advRandGetTarget", [&lua](GameObject* rawObject, int index) -> sol::object {
            auto object = typeinfo_cast<ChanceTriggerGameObject*>(rawObject);
            if (!object) return sol::nil;
            auto chance = object->m_chanceObjects.at(index);
            return sol::make_object(lua, std::tuple{chance.m_groupID, chance.m_chance});
        });
        gameObjectType.set_function("advRandSetTarget", [](GameObject* rawObject, int groupID, int chanceValue, int index) {
            auto object = typeinfo_cast<ChanceTriggerGameObject*>(rawObject);
            if (!object) return;
            auto chance = object->m_chanceObjects.at(index);
            chance.m_groupID = groupID;
            chance.m_oldGroupID = chance.m_groupID;
            chance.m_chance = chanceValue;
        });
        gameObjectType.set_function("advRandAddTarget", [](GameObject* rawObject, int groupID, int chanceValue, sol::optional<int> index) {
            auto object = typeinfo_cast<ChanceTriggerGameObject*>(rawObject);
            if (!object) return;
            auto chance = ChanceObject{groupID, groupID, chanceValue, 0};
            auto chances = object->m_chanceObjects;
            if (index.has_value())
                chances.insert(chances.begin() + index.value(), chance);
            else
                chances.push_back(chance);
        });
        gameObjectType.set_function("advRandRemoveTarget", [](GameObject* rawObject, int index) {
            auto object = typeinfo_cast<ChanceTriggerGameObject*>(rawObject);
            if (!object) return;
            auto chances = object->m_chanceObjects;
            chances.erase(chances.begin() + index);
        });
    }
}