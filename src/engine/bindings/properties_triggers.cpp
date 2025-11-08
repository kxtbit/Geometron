#include "properties.hpp"

void addPropertiesForTriggers(sol::state_view& lua, EditorUI* self, sol::usertype<GameObject> gameObjectType) {
#define SUBCLASS_PROPERTY(type, name) gameObjectType[#name] = gameObjectSubclassProperty<&type::m_##name>()
#define SUBCLASS_POINT_PROPERTY(type, name) gameObjectType[#name] = gameObjectSubclassPointProperty<&type::m_##name>()
#define SUBCLASS_READONLY_PROPERTY(type, name) gameObjectType[#name] = gameObjectSubclassReadOnlyProperty<&type::m_##name>()
#define SUBCLASS_READONLY_POINT_PROPERTY(type, name) gameObjectType[#name] = gameObjectSubclassReadOnlyPointProperty<&type::m_##name>()
#define SUBCLASS_HOOKED_PROPERTY(type, name, hook) gameObjectType[#name] = gameObjectSubclassProperty<&type::m_##name, hook>()

    { //trigger parameters
        //used to signal the editor to update the label for a trigger when it should change
        WriteHook(GameObject, labelHook, {
            if (!gameObjectExists(object)) return;
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

        SUBCLASS_PROPERTY(EffectGameObject, triggerTargetColor);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, duration, durationHook::f);
        SUBCLASS_PROPERTY(EffectGameObject, opacity);
        SUBCLASS_READONLY_PROPERTY(EffectGameObject, triggerEffectPlaying);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, targetGroupID, labelHook::f);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, centerGroupID, labelHook::f);
        SUBCLASS_PROPERTY(EffectGameObject, isTouchTriggered);
        SUBCLASS_PROPERTY(EffectGameObject, isSpawnTriggered);
        SUBCLASS_PROPERTY(EffectGameObject, hasCenterEffect);
        SUBCLASS_PROPERTY(EffectGameObject, shakeStrength);
        SUBCLASS_PROPERTY(EffectGameObject, shakeInterval);
        SUBCLASS_PROPERTY(EffectGameObject, tintGround);
        SUBCLASS_PROPERTY(EffectGameObject, usesPlayerColor1);
        SUBCLASS_PROPERTY(EffectGameObject, usesPlayerColor2);
        SUBCLASS_PROPERTY(EffectGameObject, usesBlending);
        SUBCLASS_POINT_PROPERTY(EffectGameObject, moveOffset);
        SUBCLASS_PROPERTY(EffectGameObject, easingType);
        SUBCLASS_PROPERTY(EffectGameObject, easingRate);
        SUBCLASS_PROPERTY(EffectGameObject, lockToPlayerX);
        SUBCLASS_PROPERTY(EffectGameObject, lockToPlayerY);
        SUBCLASS_PROPERTY(EffectGameObject, lockToCameraX);
        SUBCLASS_PROPERTY(EffectGameObject, lockToCameraY);
        SUBCLASS_PROPERTY(EffectGameObject, useMoveTarget);
        SUBCLASS_PROPERTY(EffectGameObject, moveTargetMode);
        SUBCLASS_PROPERTY(EffectGameObject, moveModX);
        SUBCLASS_PROPERTY(EffectGameObject, moveModY);
        SUBCLASS_PROPERTY(EffectGameObject, smallStep);
        SUBCLASS_PROPERTY(EffectGameObject, isDirectionFollowSnap360);
        SUBCLASS_PROPERTY(EffectGameObject, targetModCenterID);
        SUBCLASS_PROPERTY(EffectGameObject, directionModeDistance);
        SUBCLASS_PROPERTY(EffectGameObject, isDynamicMode);
        SUBCLASS_PROPERTY(EffectGameObject, isSilent);
        SUBCLASS_PROPERTY(EffectGameObject, specialTarget);
        SUBCLASS_PROPERTY(EffectGameObject, rotationDegrees);
        SUBCLASS_PROPERTY(EffectGameObject, times360);
        SUBCLASS_PROPERTY(EffectGameObject, lockObjectRotation);
        SUBCLASS_PROPERTY(EffectGameObject, rotationTargetID);
        SUBCLASS_PROPERTY(EffectGameObject, rotationOffset);
        SUBCLASS_PROPERTY(EffectGameObject, dynamicModeEasing);
        SUBCLASS_PROPERTY(EffectGameObject, followXMod);
        SUBCLASS_PROPERTY(EffectGameObject, followYMod);
        SUBCLASS_PROPERTY(EffectGameObject, followYSpeed);
        SUBCLASS_PROPERTY(EffectGameObject, followYDelay);
        SUBCLASS_PROPERTY(EffectGameObject, followYOffset);
        SUBCLASS_PROPERTY(EffectGameObject, followYMaxSpeed);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, fadeInDuration, durationHook::f);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, holdDuration, durationHook::f);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, fadeOutDuration, durationHook::f);
        SUBCLASS_PROPERTY(EffectGameObject, pulseMode);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, pulseTargetType, labelHook::f);
        SUBCLASS_PROPERTY(EffectGameObject, hsvValue);
        SUBCLASS_PROPERTY(EffectGameObject, copyColorID);
        SUBCLASS_PROPERTY(EffectGameObject, copyOpacity);
        SUBCLASS_PROPERTY(EffectGameObject, pulseMainOnly);
        SUBCLASS_PROPERTY(EffectGameObject, pulseDetailOnly);
        SUBCLASS_PROPERTY(EffectGameObject, pulseExclusive);
        SUBCLASS_PROPERTY(EffectGameObject, legacyHSV);
        SUBCLASS_PROPERTY(EffectGameObject, activateGroup);
        SUBCLASS_PROPERTY(EffectGameObject, touchHoldMode);
        SUBCLASS_PROPERTY(EffectGameObject, touchToggleMode);
        SUBCLASS_PROPERTY(EffectGameObject, touchPlayerMode);
        SUBCLASS_PROPERTY(EffectGameObject, isDualMode);
        SUBCLASS_PROPERTY(EffectGameObject, animationID);
        SUBCLASS_PROPERTY(EffectGameObject, spawnXPosition);
        SUBCLASS_PROPERTY(EffectGameObject, spawnOrder);
        SUBCLASS_PROPERTY(EffectGameObject, isMultiTriggered);
        SUBCLASS_PROPERTY(EffectGameObject, previewDisable);
        SUBCLASS_PROPERTY(EffectGameObject, spawnOrdered);
        SUBCLASS_PROPERTY(EffectGameObject, triggerOnExit);
        SUBCLASS_PROPERTY(EffectGameObject, itemID2);
        SUBCLASS_PROPERTY(EffectGameObject, controlID);
        SUBCLASS_PROPERTY(EffectGameObject, targetControlID);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, isDynamicBlock, labelHook::f);
        SUBCLASS_HOOKED_PROPERTY(EffectGameObject, itemID, labelHook::f);
        SUBCLASS_PROPERTY(EffectGameObject, targetPlayer1);
        SUBCLASS_PROPERTY(EffectGameObject, targetPlayer2);
        SUBCLASS_PROPERTY(EffectGameObject, followCPP);
        SUBCLASS_PROPERTY(EffectGameObject, subtractCount);
        SUBCLASS_PROPERTY(EffectGameObject, collectibleIsPickupItem);
        SUBCLASS_PROPERTY(EffectGameObject, collectibleIsToggleTrigger);
        SUBCLASS_PROPERTY(EffectGameObject, collectibleParticleID);
        SUBCLASS_PROPERTY(EffectGameObject, collectiblePoints);
        SUBCLASS_PROPERTY(EffectGameObject, hasNoAnimation);
        SUBCLASS_PROPERTY(EffectGameObject, forceModID);
        SUBCLASS_PROPERTY(EffectGameObject, rotateFollowP1);
        SUBCLASS_PROPERTY(EffectGameObject, rotateFollowP2);
        SUBCLASS_PROPERTY(EffectGameObject, gravityValue);
        SUBCLASS_PROPERTY(EffectGameObject, isSinglePTouch);
        SUBCLASS_PROPERTY(EffectGameObject, zoomValue);
        SUBCLASS_PROPERTY(EffectGameObject, cameraIsFreeMode);
        SUBCLASS_PROPERTY(EffectGameObject, cameraEditCameraSettings);
        SUBCLASS_PROPERTY(EffectGameObject, cameraEasingValue);
        SUBCLASS_PROPERTY(EffectGameObject, cameraPaddingValue);
        SUBCLASS_PROPERTY(EffectGameObject, cameraDisableGridSnap);
        SUBCLASS_PROPERTY(EffectGameObject, endReversed);
        SUBCLASS_PROPERTY(EffectGameObject, timeWarpTimeMod);
        gameObjectType["shouldPreview"] = sol::property([](GameObject* object, sol::this_state lua) -> sol::object {
            return sol::make_object(lua, subclassCast<EffectGameObject>(object, lua)->m_shouldPreview);
        }, [](GameObject* object, bool shouldPreview, curengine engine, sol::this_state lua) {
            auto effect = subclassCast<EffectGameObject>(object, lua);
            effect->m_shouldPreview = shouldPreview;
            if (!gameObjectExists(object)) return;
            engine->editor->m_editorLayer->tryUpdateSpeedObject(effect, false);
        });
        SUBCLASS_PROPERTY(EffectGameObject, ordValue);
        SUBCLASS_PROPERTY(EffectGameObject, channelValue);
        SUBCLASS_PROPERTY(EffectGameObject, isReverse);
        SUBCLASS_READONLY_PROPERTY(EffectGameObject, speedModType);
        SUBCLASS_READONLY_POINT_PROPERTY(EffectGameObject, speedStart);
        SUBCLASS_READONLY_PROPERTY(EffectGameObject, secretCoinID);
        SUBCLASS_READONLY_POINT_PROPERTY(EffectGameObject, endPosition);
        SUBCLASS_PROPERTY(EffectGameObject, spawnTriggerDelay);
        SUBCLASS_PROPERTY(EffectGameObject, gravityMod);
        SUBCLASS_PROPERTY(EffectGameObject, ignoreGroupParent);
        SUBCLASS_PROPERTY(EffectGameObject, ignoreLinkedObjects);
        SUBCLASS_READONLY_PROPERTY(EffectGameObject, channelChanged);

        SUBCLASS_PROPERTY(EffectGameObject, isSinglePTouch);
        SUBCLASS_PROPERTY(EffectGameObject, hasCenterEffect);
        SUBCLASS_PROPERTY(EffectGameObject, isReverse);
        SUBCLASS_PROPERTY(EffectGameObject, controlID);


        SUBCLASS_PROPERTY(CameraTriggerGameObject, exitStatic);
        SUBCLASS_PROPERTY(CameraTriggerGameObject, followObject);
        SUBCLASS_PROPERTY(CameraTriggerGameObject, followEasing);
        SUBCLASS_PROPERTY(CameraTriggerGameObject, edgeDirection);
        SUBCLASS_PROPERTY(CameraTriggerGameObject, smoothVelocity);
        SUBCLASS_PROPERTY(CameraTriggerGameObject, velocityModifier);
        SUBCLASS_PROPERTY(CameraTriggerGameObject, exitInstant);
        SUBCLASS_PROPERTY(CameraTriggerGameObject, previewOpacity);

        SUBCLASS_PROPERTY(CountTriggerGameObject, pickupCount);
        SUBCLASS_PROPERTY(CountTriggerGameObject, pickupTriggerMode);
        SUBCLASS_PROPERTY(CountTriggerGameObject, multiActivate);
        SUBCLASS_PROPERTY(CountTriggerGameObject, isOverride);
        SUBCLASS_PROPERTY(CountTriggerGameObject, pickupTriggerMultiplier);

        //why so many properties for advanced follow trigger aaaa
        {
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, delay);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, delayVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, startSpeed);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, startSpeedVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, startSpeedReference);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, startDirection);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, startDirectionVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, startDirectionReference);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, maxSpeed);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, maxSpeedVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, xOnly);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, yOnly);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, maxRange);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, maxRangeVariance);
            //gameObjectType["property310"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property310>();
            //gameObjectType["property311"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property311>();
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, acceleration);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, accelerationVariance);
            //gameObjectType["property312"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property312>();
            //gameObjectType["property313"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property313>();
            //gameObjectType["property314"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property314>();
            //gameObjectType["property315"] = gameObjectSubclassProperty<&AdvancedFollowTriggerObject::m_property315>();
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, steerForce);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, steerForceVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, steerForceLowEnabled);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, steerForceLow);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, steerForceLowVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, steerForceHighEnabled);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, steerForceHigh);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, steerFroceHighVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, speedRangeLow);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, speedRangeLowVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, speedRangeHigh);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, speedRangeHighVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, breakForce);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, breakForceVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, breakAngle);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, breakAngleVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, breakSteerForce);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, breakSteerForceVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, breakSteerSpeedLimit);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, breakSteerSpeedLimitVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, targetDirection);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, ignoreDisabled);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, rotateDirection);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, rotationOffset);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, nearAcceleration);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, nearAccelerationVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, nearDistance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, nearDistanceVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, nearFriction);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, nearFrictionVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, friction);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, frictionVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, easing);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, easingVariance);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, rotateEasing);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, rotateDeadZ);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, priority);
            //gameObjectType["unk7fc"] = gameObjectSubclassProperty(lua, &AdvancedFollowTriggerObject::m_unk7fc);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, maxRangeReference);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, followMode);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, exclusive);
            SUBCLASS_PROPERTY(AdvancedFollowTriggerObject, startMode);
        }

        SUBCLASS_PROPERTY(AdvancedFollowEditObject, modX);
        SUBCLASS_PROPERTY(AdvancedFollowEditObject, modXVariance);
        SUBCLASS_PROPERTY(AdvancedFollowEditObject, modY);
        SUBCLASS_PROPERTY(AdvancedFollowEditObject, modYVariance);
        SUBCLASS_PROPERTY(AdvancedFollowEditObject, redirectDirection);

        SUBCLASS_PROPERTY(EndTriggerGameObject, noEffects);
        SUBCLASS_PROPERTY(EndTriggerGameObject, noSFX);
        SUBCLASS_PROPERTY(EndTriggerGameObject, instant);

        SUBCLASS_PROPERTY(EnterEffectObject, enterType);
        SUBCLASS_PROPERTY(EnterEffectObject, length);
        SUBCLASS_PROPERTY(EnterEffectObject, lengthVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, offset);
        SUBCLASS_PROPERTY(EnterEffectObject, offsetVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, offsetY);
        SUBCLASS_PROPERTY(EnterEffectObject, offsetYVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, moveDistance);
        SUBCLASS_PROPERTY(EnterEffectObject, moveDistanceVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, areaScaleX);
        SUBCLASS_PROPERTY(EnterEffectObject, areaScaleXVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, areaScaleY);
        SUBCLASS_PROPERTY(EnterEffectObject, areaScaleYVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, moveAngle);
        SUBCLASS_PROPERTY(EnterEffectObject, moveAngleVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, startAngle);
        SUBCLASS_POINT_PROPERTY(EnterEffectObject, anglePosition);
        SUBCLASS_PROPERTY(EnterEffectObject, relative);
        SUBCLASS_PROPERTY(EnterEffectObject, relativeFade);
        SUBCLASS_PROPERTY(EnterEffectObject, easingInType);
        SUBCLASS_PROPERTY(EnterEffectObject, easingInRate);
        SUBCLASS_PROPERTY(EnterEffectObject, easingInBuffer);
        SUBCLASS_PROPERTY(EnterEffectObject, easingOutType);
        SUBCLASS_PROPERTY(EnterEffectObject, easingOutRate);
        SUBCLASS_PROPERTY(EnterEffectObject, easingOutBuffer);
        SUBCLASS_PROPERTY(EnterEffectObject, moveX);
        SUBCLASS_PROPERTY(EnterEffectObject, moveXVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, moveY);
        SUBCLASS_PROPERTY(EnterEffectObject, moveYVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, tintChannelID);
        //gameObjectType["property224"] = gameObjectSubclassProperty<&EnterEffectObject::m_property224>();
        SUBCLASS_PROPERTY(EnterEffectObject, directionType);
        SUBCLASS_PROPERTY(EnterEffectObject, xyMode);
        SUBCLASS_PROPERTY(EnterEffectObject, easeOutEnabled);
        SUBCLASS_PROPERTY(EnterEffectObject, modFront);
        SUBCLASS_PROPERTY(EnterEffectObject, modBack);
        SUBCLASS_PROPERTY(EnterEffectObject, areaTint);
        //gameObjectType["property285"] = gameObjectSubclassProperty<&EnterEffectObject::m_property285>();
        SUBCLASS_PROPERTY(EnterEffectObject, effectID);
        SUBCLASS_PROPERTY(EnterEffectObject, areaRotation);
        SUBCLASS_PROPERTY(EnterEffectObject, areaRotationVariance);
        SUBCLASS_PROPERTY(EnterEffectObject, toOpacity);
        SUBCLASS_PROPERTY(EnterEffectObject, fromOpacity);
        SUBCLASS_PROPERTY(EnterEffectObject, inbound);
        SUBCLASS_PROPERTY(EnterEffectObject, hsvEnabled);
        SUBCLASS_PROPERTY(EnterEffectObject, deadzone);
        SUBCLASS_PROPERTY(EnterEffectObject, twoDirections);
        SUBCLASS_PROPERTY(EnterEffectObject, dontEditAreaParent);
        SUBCLASS_PROPERTY(EnterEffectObject, priority);
        //gameObjectType["unk7d8"] = gameObjectSubclassProperty(lua, &EnterEffectObject::m_unk7d8);
        SUBCLASS_PROPERTY(EnterEffectObject, enterChannel);
        SUBCLASS_PROPERTY(EnterEffectObject, useEffectID);
        //gameObjectType["unk7e4"] = gameObjectSubclassProperty(lua, &EnterEffectObject::m_unk7e4);
        //gameObjectType["unk7ec"] = gameObjectSubclassProperty(lua, &EnterEffectObject::m_unk7ec);
        SUBCLASS_PROPERTY(EnterEffectObject, negativeTargetX);
        SUBCLASS_PROPERTY(EnterEffectObject, areaRange);
        //gameObjectType["unk7fc"] = gameObjectSubclassProperty(lua, &EnterEffectObject::m_unk7fc);

        SUBCLASS_PROPERTY(EventLinkTrigger, resetRemap);
        SUBCLASS_PROPERTY(EventLinkTrigger, extraID);
        SUBCLASS_PROPERTY(EventLinkTrigger, extraID2);
        gameObjectType["eventIDCount"] = sol::readonly_property([](GameObject* object, sol::this_state lua) -> sol::object {
            return sol::make_object(lua, static_cast<int>(subclassCast<EventLinkTrigger>(object, lua)->m_eventIDs.size()));
        });
        gameObjectType.set_function("eventGetIDs", [](GameObject* object, sol::this_state lua) -> sol::object {
            auto eventIDs = subclassCast<EventLinkTrigger>(object, lua)->m_eventIDs;
            auto table = sol::table::create(lua.lua_state(), eventIDs.size());
            int i = 1;
            for (int id : eventIDs) {
                table[i++] = id;
            }
            return table;
        });
        gameObjectType.set_function("eventAddID", [](GameObject* rawObject, sol::variadic_args ids, sol::this_state lua) {
            auto object = subclassCast<EventLinkTrigger>(rawObject, lua);
            for (sol::optional<int> id : ids) {
                if (!id.has_value()) continue;
                object->m_eventIDs.erase(id.value());
            }
        });
        gameObjectType.set_function("eventSetID", [](GameObject* rawObject, sol::variadic_args ids, sol::this_state lua) {
            auto object = subclassCast<EventLinkTrigger>(rawObject, lua);
            object->m_eventIDs.clear();
            for (sol::optional<int> id : ids) {
                if (!id.has_value()) continue;
                object->m_eventIDs.erase(id.value());
            }
        });
        gameObjectType.set_function("eventRemoveID", [](GameObject* rawObject, sol::variadic_args ids, sol::this_state lua) {
            auto object = subclassCast<EventLinkTrigger>(rawObject, lua);
            for (sol::optional<int> id : ids) {
                if (!id.has_value()) continue;
                object->m_eventIDs.extract(id.value());
            }
        });

        SUBCLASS_PROPERTY(ForceBlockGameObject, force);
        SUBCLASS_PROPERTY(ForceBlockGameObject, minForce);
        SUBCLASS_PROPERTY(ForceBlockGameObject, maxForce);
        SUBCLASS_PROPERTY(ForceBlockGameObject, relativeForce);
        SUBCLASS_PROPERTY(ForceBlockGameObject, forceRange);
        SUBCLASS_PROPERTY(ForceBlockGameObject, forceID);

        SUBCLASS_PROPERTY(GameOptionsTrigger, streakAdditive);
        SUBCLASS_PROPERTY(GameOptionsTrigger, unlinkDualGravity);
        SUBCLASS_PROPERTY(GameOptionsTrigger, hideGround);
        SUBCLASS_PROPERTY(GameOptionsTrigger, hideP1);
        SUBCLASS_PROPERTY(GameOptionsTrigger, hideP2);
        SUBCLASS_PROPERTY(GameOptionsTrigger, disableP1Controls);
        SUBCLASS_PROPERTY(GameOptionsTrigger, disableP2Controls);
        SUBCLASS_PROPERTY(GameOptionsTrigger, hideMG);
        SUBCLASS_PROPERTY(GameOptionsTrigger, hideAttempts);
        SUBCLASS_PROPERTY(GameOptionsTrigger, editRespawnTime);
        SUBCLASS_PROPERTY(GameOptionsTrigger, respawnTime);
        SUBCLASS_PROPERTY(GameOptionsTrigger, audioOnDeath);
        SUBCLASS_PROPERTY(GameOptionsTrigger, noDeathSFX);
        SUBCLASS_PROPERTY(GameOptionsTrigger, boostSlide);

        SUBCLASS_PROPERTY(GradientTriggerObject, blendingLayer);
        SUBCLASS_PROPERTY(GradientTriggerObject, blendingMode);
        SUBCLASS_PROPERTY(GradientTriggerObject, gradientID);
        SUBCLASS_PROPERTY(GradientTriggerObject, upBottomLeftID);
        SUBCLASS_PROPERTY(GradientTriggerObject, downBottomRightID);
        SUBCLASS_PROPERTY(GradientTriggerObject, leftTopLeftID);
        SUBCLASS_PROPERTY(GradientTriggerObject, rightTopRightID);
        SUBCLASS_PROPERTY(GradientTriggerObject, vertexMode);
        SUBCLASS_PROPERTY(GradientTriggerObject, disable);
        SUBCLASS_PROPERTY(GradientTriggerObject, disableAll);
        SUBCLASS_PROPERTY(GradientTriggerObject, previewOpacity);

        SUBCLASS_PROPERTY(ItemTriggerGameObject, item1Mode);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, item2Mode);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, targetItemMode);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, mod1);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, mod2);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, resultType1);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, resultType2);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, resultType3);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, tolerance);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, roundType1);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, roundType2);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, signType1);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, signType2);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, persistent);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, targetAll);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, reset);
        SUBCLASS_PROPERTY(ItemTriggerGameObject, timer);

        SUBCLASS_PROPERTY(KeyframeAnimTriggerObject, timeMod);
        SUBCLASS_PROPERTY(KeyframeAnimTriggerObject, positionXMod);
        SUBCLASS_PROPERTY(KeyframeAnimTriggerObject, positionYMod);
        SUBCLASS_PROPERTY(KeyframeAnimTriggerObject, rotationMod);
        SUBCLASS_PROPERTY(KeyframeAnimTriggerObject, scaleXMod);
        SUBCLASS_PROPERTY(KeyframeAnimTriggerObject, scaleYMod);

        SUBCLASS_READONLY_PROPERTY(KeyframeGameObject, keyframeGroup);
        SUBCLASS_READONLY_PROPERTY(KeyframeGameObject, keyframeIndex);
        SUBCLASS_PROPERTY(KeyframeGameObject, referenceOnly);
        SUBCLASS_PROPERTY(KeyframeGameObject, proximity);
        SUBCLASS_PROPERTY(KeyframeGameObject, curve);
        SUBCLASS_PROPERTY(KeyframeGameObject, closeLoop);
        SUBCLASS_PROPERTY(KeyframeGameObject, timeMode);
        //gameObjectType["unk760"] = gameObjectSubclassProperty(lua, &KeyframeGameObject::m_unk760);
        SUBCLASS_PROPERTY(KeyframeGameObject, spawnDelay);
        SUBCLASS_PROPERTY(KeyframeGameObject, previewArt);
        SUBCLASS_PROPERTY(KeyframeGameObject, keyframeActive);
        SUBCLASS_PROPERTY(KeyframeGameObject, autoLayer);
        SUBCLASS_PROPERTY(KeyframeGameObject, direction);
        SUBCLASS_PROPERTY(KeyframeGameObject, revolutions);
        SUBCLASS_PROPERTY(KeyframeGameObject, lineOpacity);

        SUBCLASS_PROPERTY(TimerTriggerGameObject, startTime);
        SUBCLASS_PROPERTY(TimerTriggerGameObject, targetTime);
        SUBCLASS_PROPERTY(TimerTriggerGameObject, stopTimeEnabled);
        SUBCLASS_PROPERTY(TimerTriggerGameObject, dontOverride);
        SUBCLASS_PROPERTY(TimerTriggerGameObject, ignoreTimeWarp);
        SUBCLASS_PROPERTY(TimerTriggerGameObject, timeMod);
        SUBCLASS_PROPERTY(TimerTriggerGameObject, startPaused);
        SUBCLASS_PROPERTY(TimerTriggerGameObject, multiActivate);
        SUBCLASS_PROPERTY(TimerTriggerGameObject, controlType);

        SUBCLASS_PROPERTY(TransformTriggerGameObject, objectScaleX);
        SUBCLASS_PROPERTY(TransformTriggerGameObject, objectScaleY);
        //gameObjectType["property450"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_property450>();
        //gameObjectType["property451"] = gameObjectSubclassProperty<&TransformTriggerGameObject::m_property451>();
        SUBCLASS_PROPERTY(TransformTriggerGameObject, onlyMove);
        SUBCLASS_PROPERTY(TransformTriggerGameObject, divideX);
        SUBCLASS_PROPERTY(TransformTriggerGameObject, divideY);
        SUBCLASS_PROPERTY(TransformTriggerGameObject, relativeRotation);
        SUBCLASS_PROPERTY(TransformTriggerGameObject, relativeScale);

        gameObjectType["advRandTargetCount"] = sol::readonly_property([](GameObject* object, sol::this_state lua) -> sol::object {
            return sol::make_object(lua, static_cast<int>(subclassCast<ChanceTriggerGameObject>(object, lua)->m_chanceObjects.size()));
        });
        gameObjectType.set_function("advRandGetTarget", [](GameObject* object, int index, sol::this_state lua) -> sol::object {
            auto chance = subclassCast<ChanceTriggerGameObject>(object, lua)->m_chanceObjects.at(index);
            return sol::make_object(lua, std::tuple{chance.m_groupID, chance.m_chance});
        });
        gameObjectType.set_function("advRandSetTarget", [](GameObject* object, int groupID, int chanceValue, int index, sol::this_state lua) {
            auto chance = subclassCast<ChanceTriggerGameObject>(object, lua)->m_chanceObjects.at(index);
            chance.m_groupID = groupID;
            chance.m_oldGroupID = chance.m_groupID;
            chance.m_chance = chanceValue;
        });
        gameObjectType.set_function("advRandAddTarget", [](GameObject* object, int groupID, int chanceValue, sol::optional<int> index, sol::this_state lua) {
            auto chance = ChanceObject(groupID, chanceValue);
            auto chances = subclassCast<ChanceTriggerGameObject>(object, lua)->m_chanceObjects;
            if (index.has_value())
                chances.insert(chances.begin() + index.value(), chance);
            else
                chances.push_back(chance);
        });
        gameObjectType.set_function("advRandRemoveTarget", [](GameObject* object, int index, sol::this_state lua) {
            auto chances = subclassCast<ChanceTriggerGameObject>(object, lua)->m_chanceObjects;
            chances.erase(chances.begin() + index);
        });
    }
}