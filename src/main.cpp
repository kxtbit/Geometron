// ReSharper disable CppHidingFunction
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppFunctionalStyleCast
#include <Geode/Geode.hpp>
#include <geode.custom-keybinds/include/OptionalAPI.hpp>

#include "engine/LuaEngine.hpp"

#include "ui/ScriptSelectorPopup.hpp"

using namespace geode::prelude;

#include <Geode/modify/EditorUI.hpp>
class $modify(GeometronEditorUI, EditorUI) {
    static void luaPanic(sol::optional<std::string> msg) {
        if (msg) {
            log::error("Lua panic triggered: {}", msg.value());
        } else {
            log::error("Lua panic triggered with no error info");
        }
        *reinterpret_cast<volatile int*>(0) = 1;
    }

    $override
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        using namespace keybinds;
        this->addEventListener<InvokeBindFilterV2>([this](InvokeBindEventV2* event) {
            if (!event->isDown()) return ListenerResult::Propagate;

            auto scene = CCScene::get();
            CCArrayExt<CCNode*> children = scene->getChildren();
            for (auto child : children) {
                //do not open the script selector again if it is already open
                if (child->getUserObject("is-script-selector-popup")) return ListenerResult::Propagate;
            }

            auto popup = ScriptSelectorPopup::create(this);
            if (!popup) {
                log::error("Could not initialize script selector");
            } else {
                popup->m_noElasticity = true;
                popup->show();
            }
            return ListenerResult::Propagate;
        }, "script-menu"_spr);

        return true;
    }

    void onScriptButton(CCObject* object) {
        auto popup = ScriptSelectorPopup::create(this);
        if (!popup) {
            log::error("Could not initialize script selector");
            return;
        }
        popup->m_noElasticity = true;
        popup->show();
        /*auto dir = Mod::get()->getSaveDir();
        auto result = file::readString(dir / "testscript.lua");
        if (!result.ok()) {
            log::error("Failed to read testscript.lua: {}", result.err().value_or("unknown"));
            return;
        }
        runLuaScript(result.ok().value());*/
    }

    $override
    void createMoveMenu() {
        EditorUI::createMoveMenu();

        auto button = this->getSpriteButton("script_button.png"_spr,
            menu_selector(GeometronEditorUI::onScriptButton),
            nullptr, 0.9f);
        button->setID("script-button"_spr);
        m_editButtonBar->m_buttonArray->addObject(button);

        auto rows = GameManager::sharedState()->getIntGameVariable("0049");
        auto columns = GameManager::sharedState()->getIntGameVariable("0050");
        m_editButtonBar->reloadItems(rows, columns);
    }
};

#include <Geode/modify/GameManager.hpp>
class $modify(GameManager) {
    $override
    void returnToLastScene(GJGameLevel* level) {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (scene->getChildByType<LevelEditorLayer>(0) != nullptr) {
            auto engine = LuaEngine::get();
            if (engine->editor != nullptr) {
                //log::info("must reset engine");
                engine->forceReset();
            }
        }
        GameManager::returnToLastScene(level);
    }
};