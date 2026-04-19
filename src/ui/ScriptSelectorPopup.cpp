// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppHidingFunction
#include <Geode/Geode.hpp>

#include "ScriptSelectorPopup.hpp"

#include "../engine/LuaEngine.hpp"
#include "EnhancedTextPanel.hpp"
#include "ScrollingWindow.hpp"

#include "../utils/ScriptLoader.hpp"
#include "../utils/Settings.hpp"

using namespace geode::prelude;

volatile int lastTabIndex = 0;

//i tried using enterPressed on TextInputDelegate but it no worky :(
#include <Geode/modify/CCKeyboardDispatcher.hpp>
class $modify(ConsoleKeyboardHook, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double idk) {
        if (key == KEY_Enter && isKeyDown) {
            auto console = LuaEngine::get()->getCurrentConsole();
            if (console != nullptr && console->consoleTab->isVisible()) {
                console->sendConsoleInput(nullptr);
                return true;
            }
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, idk);
    }
};

inline static float FONT_SIZE_MULT = 0.6f;
inline static std::string FONT_NAME = "geode.loader/mdFontMono.fnt";//"AdwaitaMono-Regular.fnt"_spr;
bool ScriptSelectorPopup::init(EditorUI* editor) {
    if (!Popup::init(380.0f, 280.0f))
        return false;

    setUserObject("is-script-selector-popup", CCBool::create(true));

    this->editor = editor;

    auto popupSize = m_mainLayer->getContentSize();

    auto scriptLoader = ScriptLoader::get();
    auto scripts = scriptLoader->getScripts();
    if (!scriptLoader->isLoaded()) {
        errorMessage = "ScriptLoader failed to initialize properly.\nCheck the logs for more details.";
        return true;
    }
    engine = LuaEngine::get();
    if (!engine) {
        errorMessage = "LuaEngine failed to initialize properly.\nCheck the logs for more details.";
        return true;
    }

    tabMenu = CCMenu::create();
    tabMenu->setPosition({popupSize.width / 2, popupSize.height - 22});
    tabMenu->setContentSize({popupSize.width - 80, 25});
    tabMenu->setZOrder(1);
    //this seems to work i guess???
    tabMenu->setTouchPriority(-503);
    tabMenu->setID("tab-menu"_spr);

    scriptTabButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Scripts", 100, 0, 0.55, true, "bigFont.fnt", "GJ_button_04.png", 24.0),
        this, menu_selector(ScriptSelectorPopup::onClickTab));
    scriptTabButton->setTag(0);
    scriptTabButton->setID("tab-button-scripts"_spr);
    consoleTabButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Console", 100, 0, 0.55, true, "bigFont.fnt", "GJ_button_04.png", 24.0),
        this, menu_selector(ScriptSelectorPopup::onClickTab));
    consoleTabButton->setTag(1);
    consoleTabButton->setID("tab-button-console"_spr);

    tabMenu->addChild(scriptTabButton);
    tabMenu->addChild(consoleTabButton);
    tabMenu->setLayout(RowLayout::create());
    m_mainLayer->addChild(tabMenu);

    scriptTab = CCMenu::create();
    scriptTab->setContentSize(popupSize);
    scriptTab->setPosition(popupSize / 2);
    scriptTab->ignoreAnchorPointForPosition(false);
    scriptTab->setID("tab-container-scripts"_spr);
    consoleTab = CCMenu::create();
    consoleTab->setContentSize(popupSize);
    consoleTab->setPosition(popupSize / 2);
    consoleTab->ignoreAnchorPointForPosition(false);
    consoleTab->setID("tab-container-console"_spr);

    //both the script tab and the console tab have scrolling layers and use the scroll wheel
    //if both are present at the same time the scrolling layers would fight for input causing weird issues
    //so when one tab is active the other is removed, meaning both tabs should be manually retained
    scriptTab->retain();
    consoleTab->retain();

    setTab(lastTabIndex);
    {
        auto listSize = CCSize(300.0f, 200.0f);
        auto listPosition = CCPoint(popupSize / 2);
        auto rowSize = CCSize(listSize.width, 50);

        CCArrayExt<CCMenu*> rows;
        for (int i = 0; i < scripts.size(); i++) {
            auto [name, path] = scripts[i];

            auto row = CCMenu::create();
            row->setContentSize(rowSize);
            row->setID("script-row"_spr);

            auto label = CCLabelBMFont::create(name.c_str(), "chatFont.fnt");
            label->setAnchorPoint({0, 0.5});
            label->setPosition({15, rowSize.height / 2});
            label->setScale(0.5);
            label->setID("script-label"_spr);

            auto buttonSprite = ButtonSprite::create("Run", 50, false, "bigFont.fnt", "GJ_button_01.png", 30, 1.0f);
            auto button = CCMenuItemSpriteExtra::create(buttonSprite, this, menu_selector(ScriptSelectorPopup::onClickRun));
            //button->setAnchorPoint({1, 0.5});
            button->setTag(i);
            button->setPosition({rowSize.width - 15 - button->getContentSize().width / 2, rowSize.height / 2});
            button->setID("script-run-button"_spr);

            row->addChild(label);
            row->addChild(button);

            rows.push_back(row);
        }

        list = ListView::create(rows.inner(), rowSize.height, listSize.width, listSize.height);
        list->setPosition(listPosition);
        list->ignoreAnchorPointForPosition(false);
        list->setID("script-list"_spr);

        listBorders = ListBorders::create();
        listBorders->setContentSize(listSize + CCSize(3, 5));
        listBorders->setPosition(listPosition);
        listBorders->setZOrder(1); //show above list
        listBorders->setID("script-list-border"_spr);

        scriptTab->addChild(list);
        scriptTab->addChild(listBorders);
    }
    {
        auto consoleSize = CCSize(300.0f, 200.0f);
        auto consolePosition = CCPoint(popupSize / 2);
        auto consoleOutSize = CCSize(consoleSize.width, consoleSize.height - 35);
        auto consoleOutOffset = (consoleSize - consoleOutSize) / 2;
        auto consoleInSize = CCSize(consoleSize.width, 30);
        auto consoleInOffset = -(consoleSize - consoleInSize) / 2;

        consoleBG = CCScale9Sprite::create("square02b_001.png");
        consoleBG->setScale(0.5f);
        consoleBG->setColor({0, 0, 0});
        consoleBG->setOpacity(90);
        consoleBG->setContentSize(consoleOutSize * 2);
        consoleBG->setZOrder(-1);
        consoleBG->setID("console-background"_spr);
        consoleTab->addChildAtPosition(consoleBG, Anchor::Center, consoleOutOffset);

        consoleWindow = ScrollingWindow::create(consoleOutSize - CCSize(20, 20));
        consoleWindow->ignoreAnchorPointForPosition(false);
        consoleWindow->setID("console-output-pane"_spr);

        consoleText = EnhancedTextPanel::create(consoleWindow,
            12 * Settings::consoleLineHeightMultiplier() * Settings::consoleFontSize(),
            FONT_NAME,
            FONT_SIZE_MULT * Settings::consoleFontSize());
        consoleText->setAnchorPoint({0, 0});
        consoleText->ignoreAnchorPointForPosition(false);
        consoleText->setContentSize(consoleOutSize - CCSize(20, 20));
        consoleText->setID("console-output-text"_spr);

        consoleWindow->addInnerChild(consoleText);
        consoleTab->addChildAtPosition(consoleWindow, Anchor::Center, consoleOutOffset);

        consoleInput = TextInput::create(consoleInSize.width - 40, "", FONT_NAME);
        consoleInput->setCommonFilter(CommonFilter::Any);
        consoleInput->setTextAlign(TextInputAlign::Left);
        consoleInput->setDelegate(this);
        consoleInput->setCallbackEnabled(false);
        consoleInput->setID("console-input"_spr);

        consoleSendButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(">", 30, false, "bigFont.fnt", "GJ_button_01.png", 30, 1.0f),
            this, menu_selector(ScriptSelectorPopup::sendConsoleInput));
        consoleSendButton->setID("console-send-button"_spr);

        consoleTab->addChildAtPosition(consoleSendButton, Anchor::Center, consoleInOffset + CCPoint((consoleInSize.width - 40) / 2.0f + 3, 0));
        consoleTab->addChildAtPosition(consoleInput, Anchor::Center, consoleInOffset - CCPoint(20, 0));
    }
    if (lastTabIndex == 1 && Settings::consoleAutoFocusInput()) {
        Loader::get()->queueInMainThread([consoleInput = Ref(consoleInput)]() {
            consoleInput->focus();
        });
    }

    loadingIndicator = LoadingCircleSprite::create(1.0f);
    loadingIndicator->setScale(0.45f);
    loadingIndicator->setZOrder(1);
    loadingIndicator->setID("script-loading-indicator");

    auto cancelButtonSprite = CCSprite::createWithSpriteFrameName("GJ_stopEditorBtn_001.png");
    if (!cancelButtonSprite) {
        log::error("Cancel button sprite not present");
        return false;
    }
    cancelButtonSprite->setScale(0.7f);
    cancelButton = CCMenuItemSpriteExtra::create(cancelButtonSprite, this, menu_selector(ScriptSelectorPopup::onClickCancel));
    cancelButton->setID("script-cancel-button");

    statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
    statusLabel->setAnchorPoint({0, 0.5});
    statusLabel->setID("script-status-label");

    updateScriptStatus();

    m_mainLayer->addChildAtPosition(loadingIndicator, Anchor::BottomLeft, {55, 22.5});
    m_buttonMenu->addChildAtPosition(cancelButton, Anchor::BottomLeft, {55, 22.5});
    m_mainLayer->addChildAtPosition(statusLabel, Anchor::BottomLeft, {85, 22.5});

    //scheduleUpdate();
    return true;
}

ScriptSelectorPopup::~ScriptSelectorPopup() {
    scriptTab->release();
    consoleTab->release();
}


void ScriptSelectorPopup::update(float delta) {
    //updateScriptStatus();
}
void ScriptSelectorPopup::updateScriptStatus(bool force) {
    ScriptExecutionStatus status;
    if (engine->fastGetIsExecuting() && (status = engine->getStatus()).type != STOPPED) {
        //obviously this is a terrible hashing function but it should work well enough to detect if something changed
        bool changed = force;
        if (status.scriptName != lastScriptName) {
            lastScriptName = status.scriptName;
            changed = true;
        }
        if (status.type != lastExecutionType) {
            lastExecutionType = status.type;
            changed = true;
        }
        if (status.yieldType != lastYieldType) {
            lastYieldType = status.yieldType;
            changed = true;
        }
        if (changed) {
            lastScriptName = status.scriptName;

            loadingIndicator->setVisible(true);
            cancelButton->setVisible(true);
            std::string statusName = "(Unknown status)";
            switch (status.yieldType) {
                case PREEMPTED:
                case MANUAL:
                    statusName = "Running..."; break;
                case SLEEPING:
                    statusName = "Sleeping..."; break;
                case WAITING_FOR_CONSOLE_CHARACTERS:
                case WAITING_FOR_CONSOLE_NEWLINE:
                    statusName = "Waiting for input..."; break;
            }
            auto labelString = fmt::format("Executing \"{}\" - {}", status.scriptName, statusName);
            statusLabel->setString(labelString.c_str());
            statusLabel->setVisible(true);
        }
    } else {
        lastExecutionType = STOPPED;
        loadingIndicator->setVisible(false);
        cancelButton->setVisible(false);
        statusLabel->setVisible(false);
    }
}

void ScriptSelectorPopup::addConsoleLine(const std::string& line) {
    float logHeight = (consoleText->lineCount() + 1) * consoleText->getLineHeight();
    if (logHeight > consoleWindow->innerHeight()) {
        float newHeight = logHeight;
        consoleWindow->resizeInner(newHeight);
        consoleText->setContentHeight(newHeight);
    }

    consoleText->addLine(line);
}
void ScriptSelectorPopup::sendConsoleInput(CCObject*) {
    std::string str = consoleInput->getString();
    consoleInput->setString("");
    LuaEngine::get()->sendInputString(str + std::string("\n"));
    consoleInput->focus();
}

void ScriptSelectorPopup::setTab(int tab) const {
    CCArrayExt<CCMenuItemSpriteExtra*> buttons = tabMenu->getChildren();
    for (auto child : buttons) {
        bool enabled;
        const char* sprite;
        if (child->getTag() == tab) {
            enabled = false;
            sprite = "GJ_button_01.png";
        } else {
            enabled = true;
            sprite = "GJ_button_04.png";
        }
        child->setEnabled(enabled);
        static_cast<ButtonSprite*>(child->getNormalImage())->updateBGImage(sprite);
    }
    switch (tab) {
        case 0:
            m_mainLayer->addChild(scriptTab);
            consoleTab->removeFromParentAndCleanup(false);
            break;
        case 1:
            scriptTab->removeFromParentAndCleanup(false);
            m_mainLayer->addChild(consoleTab);
            break;
        default: break;
    }
    lastTabIndex = tab;
}

void ScriptSelectorPopup::onClickTab(CCObject* object) {
    auto button = static_cast<CCMenuItemSpriteExtra*>(object);
    int index = button->getTag();
    setTab(index);
}
void ScriptSelectorPopup::onClickRun(CCObject* object) {
    auto button = static_cast<CCMenuItemSpriteExtra*>(object);
    int index = button->getTag();

    auto scriptLoader = ScriptLoader::get();
    if (!scriptLoader->isLoaded()) {
        FLAlertLayer::create("Error",
            "No scripts are loaded! (How did you even manage to make this button appear, anyway?)",
            "OK")->show();
        return;
    }
    auto scripts = scriptLoader->getScripts();
    if (index < 0 || index >= scripts.size()) {
        FLAlertLayer::create("Error",
            fmt::format("There is no script with index {}. Something in the UI code went horribly wrong!", index),
            "OK")->show();
        return;
    }
    auto script = scripts[index];
    auto data = scriptLoader->loadScript(script.name);
    if (!data.has_value()) {
        FLAlertLayer::create("Error",
            fmt::format("Failed to load {}. There may be more information in the Geode logs.", data),
            "OK")->show();
        return;
    }
    auto task = LuaEngine::get()->executeAsync(editor, data.value(), script.name);
    updateScriptStatus();

    task->setOnResult([name = script.name, engine = engine](ScriptResult result) {
        engine->updateScriptStatusForConsole();
        FLAlertLayer* popup = nullptr;
        switch (result.result) {
            case OK:
                popup = FLAlertLayer::create("Success",
                    fmt::format("The script {} executed successfully.", name),
                    "OK");
                break;
            case LUA_COMPILE_ERROR:
                popup = FLAlertLayer::create("Error",
                    fmt::format("The script {} failed to compile:\n{}", name, result.errorMessage),
                    "OK");
                break;
            case LUA_RUN_ERROR:
                popup = FLAlertLayer::create("Error",
                    fmt::format("The script {} failed to execute:\n{}", name, result.errorMessage),
                    "OK");
                break;
            case SCRIPT_CANCELED:
                /*popup = FLAlertLayer::create("Error",
                    fmt::format("The script {} was canceled.", name),
                    "OK");*/
                return;
                break;
            case ENGINE_LOCKED:
            case ENGINE_IN_USE:
                popup = FLAlertLayer::create("Error",
                    fmt::format("The Lua engine is busy."),
                    "OK");
                break;
        }
        if (!popup) {
            log::warn("Failed to create the script result textbox!");
            return;
        }
        popup->m_noElasticity = true;
        popup->show();
    });
    task->onProgress = [engine = engine](const ScriptExecutionStatus* status) {
        engine->updateScriptStatusForConsole();
    };
}

void ScriptSelectorPopup::onClickCancel(CCObject* object) {
    auto status = engine->getStatus();
    if (status.type != STOPPED && status.asyncData.isAsync)
        status.asyncData.task->cancel();
}

void ScriptSelectorPopup::onConsoleText(const std::string& text) {
    auto lineCount = consoleText->lineCount();
    auto lastLine = consoleText->getLineFromBottom(0);
    lastLine.append(text);
    //yes i know there should be a setLineFromBottom but i am too lazy
    consoleText->setLine(lineCount - 1, lastLine);
}
void ScriptSelectorPopup::onConsoleLine(const std::string& line) {
    addConsoleLine(line);
}

void ScriptSelectorPopup::onKeybindRelease() {
    if (lastTabIndex == 1 && Settings::consoleAutoFocusInput())
        consoleInput->focus();
}

void ScriptSelectorPopup::show() {
    auto& lines = LuaEngine::get()->getLines();
    for (std::string& str : lines) {
        auto trimmed = str.ends_with('\n') ? str.substr(0, str.size() - 1) : str;
        addConsoleLine(str);
    }
    LuaEngine::get()->registerConsole(this);
    Popup::show();

    if (errorMessage.has_value()) {
        createQuickPopup(
            "Error",
            errorMessage.value(),
            "OK", nullptr,
            [this](auto, bool btn2) {
                this->onClose(nullptr);
            }
        );
    }
}
void ScriptSelectorPopup::onClose(CCObject* object) {
    LuaEngine::get()->unregisterConsole(this);
    Popup::onClose(object);
}

//140 is a completely arbitrary distance to move out of the way of the software keyboard
//but idk how to do it the correct way :((((
void ScriptSelectorPopup::textInputOpened(CCTextInputNode* node) {
#ifdef GEODE_IS_MOBILE
    m_mainLayer->setPositionY(m_mainLayer->getPositionY() + 140);
#endif
}
void ScriptSelectorPopup::textInputClosed(CCTextInputNode* node) {
#ifdef GEODE_IS_MOBILE
    m_mainLayer->setPositionY(m_mainLayer->getPositionY() - 140);
#endif
}

void ScriptSelectorPopup::textInputShouldOffset(CCTextInputNode* node, float yOffset) {
    //this doesn't seem to work on Android from my testing
    //maybe it works on iOS but i don't have that :/
//#ifdef GEODE_IS_MOBILE
    //this->setPositionY(yOffset);
//#endif
}

ScriptSelectorPopup* ScriptSelectorPopup::create(EditorUI* editor) {
    auto ret = new ScriptSelectorPopup();
    if (ret->init(editor)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}