// ReSharper disable CppMemberFunctionMayBeStatic
#include "TrackingEditorLayer.hpp"

#include "TrackedGameObject.hpp"

void TrackingEditorLayer::addSpecial(GameObject* object) {
    LevelEditorLayer::addSpecial(object);
    static_cast<TrackedGameObject*>(object)->m_fields->objectExists = true;
}
void TrackingEditorLayer::removeSpecial(GameObject* object) {
    static_cast<TrackedGameObject*>(object)->m_fields->objectExists = false;
    LevelEditorLayer::removeSpecial(object);
}

