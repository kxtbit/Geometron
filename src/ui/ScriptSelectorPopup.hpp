#pragma once

#ifndef GEOMETRON_SCRIPTSELECTORPOPUP_HPP
#define GEOMETRON_SCRIPTSELECTORPOPUP_HPP

#include <Geode/Geode.hpp>

enum ScriptExecutionType : int;
enum ScriptYieldType : int;
class LuaEngine;
class EnhancedTextPanel;
class ScrollingWindow;
using namespace geode::prelude;

class ScriptSelectorPopup : public Popup, TextInputDelegate {
    Ref<EditorUI> editor;

    LuaEngine* engine;

    CCMenu* scriptTab = nullptr;
    ListView* list = nullptr;
    ListBorders* listBorders = nullptr;
    CCMenu* consoleTab = nullptr;
    CCScale9Sprite* consoleBG = nullptr;
    EnhancedTextPanel* consoleText = nullptr;
    ScrollingWindow* consoleWindow = nullptr;
    TextInput* consoleInput = nullptr;
    CCMenuItemSpriteExtra* consoleSendButton = nullptr;

    CCMenu* tabMenu = nullptr;
    CCMenuItemSpriteExtra* scriptTabButton = nullptr;
    CCMenuItemSpriteExtra* consoleTabButton = nullptr;

    CCLabelBMFont* statusLabel = nullptr;
    LoadingCircleSprite* loadingIndicator = nullptr;
    CCMenuItemSpriteExtra* cancelButton = nullptr;

    std::string lastScriptName;
    ScriptExecutionType lastExecutionType;
    ScriptYieldType lastYieldType;

    std::optional<std::string> errorMessage;

    bool init(EditorUI* editor);

    ~ScriptSelectorPopup() override;

    void update(float delta) override;

    void addConsoleLine(const std::string& line);
    void sendConsoleInput(CCObject*);

    void setTab(int tab) const;

    void onClickTab(CCObject* object);
    void onClickRun(CCObject* object);

    void onClickCancel(CCObject* object);

    friend class ConsoleKeyboardHook;

public:
    static ScriptSelectorPopup* create(EditorUI* editor);

    void updateScriptStatus(bool force = false);
    void onConsoleText(const std::string& text);
    void onConsoleLine(const std::string& line);

    void onKeybindRelease();

    void show() override;
    void onClose(CCObject*) override;

    void textInputOpened(CCTextInputNode* node) override;
    void textInputClosed(CCTextInputNode* node) override;
    void textInputShouldOffset(CCTextInputNode* node, float yOffset) override;
};

#endif //GEOMETRON_SCRIPTSELECTORPOPUP_HPP