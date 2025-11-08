// ReSharper disable CppHidingFunction
#pragma once

#ifndef GEOMETRON_TRACKINGEDITORLAYER_HPP
#define GEOMETRON_TRACKINGEDITORLAYER_HPP

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/LevelEditorLayer.hpp>
class $modify(TrackingEditorLayer, LevelEditorLayer) {
    void addSpecial(GameObject* object);
    void removeSpecial(GameObject* object);
};

#endif //GEOMETRON_TRACKINGEDITORLAYER_HPP