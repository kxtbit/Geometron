#pragma once

#ifndef GEOMETRON_TRACKEDGAMEOBJECT_HPP
#define GEOMETRON_TRACKEDGAMEOBJECT_HPP

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/GameObject.hpp>
class $modify(TrackedGameObject, GameObject) {
    struct Fields {
        bool objectExists = false;
    };
};

#endif //GEOMETRON_TRACKEDGAMEOBJECT_HPP