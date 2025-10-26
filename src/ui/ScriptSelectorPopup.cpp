// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppHidingFunction
#include <Geode/Geode.hpp>

#include <geode.custom-keybinds/include/OptionalAPI.hpp>

#include "ScriptSelectorPopup.hpp"

#include "../engine/LuaEngine.hpp"
#include "EnhancedTextPanel.hpp"
#include "ScrollingWindow.hpp"

#include "../utils/ScriptLoader.hpp"
#include "../utils/Settings.hpp"

using namespace geode::prelude;
using namespace keybinds;

volatile int lastTabIndex = 0;

/*$execute {
    auto categoryConsole = CategoryV2::create("Geometron/Console");
    (void)[&]() -> Result<> {
        GEODE_UNWRAP(BindManagerV2::registerBindable(GEODE_UNWRAP(BindableActionV2::create(
            "lua-console-send"_spr,
            "Send Lua console input",
            "Send the contents of the Lua console text box as input to the interactive console",
            { GEODE_UNWRAP(KeybindV2::create(KEY_Enter, ModifierV2::None)) },
            // Category; use slashes for specifying subcategories. See the
            // Category class for default categories
            GEODE_UNWRAP(categoryConsole)
        ))));
        return Ok();
    }();
}*/

#include <Geode/modify/CCKeyboardDispatcher.hpp>
class $modify(ConsoleKeyboardHook, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat) {
        if (key == KEY_Enter) {
            auto console = LuaEngine::get()->getCurrentConsole();
            if (console != nullptr && console->consoleTab->isVisible()) {
                console->sendConsoleInput(nullptr);
                return true;
            }
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat);
    }
};

bool ScriptSelectorPopup::setup(EditorUI* editor) {
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
        errorMessage = "LuaEngine failed to enitialize properly.\nCheck the logs for more details.";
        return true;
    }

    tabMenu = CCMenu::create();
    tabMenu->setPosition({popupSize.width / 2, popupSize.height - 22});
    tabMenu->setContentSize({popupSize.width - 80, 25});
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
    scriptTab->ignoreAnchorPointForPosition(false);
    scriptTab->setID("tab-container-scripts"_spr);
    consoleTab = CCMenu::create();
    consoleTab->setContentSize(popupSize);
    consoleTab->ignoreAnchorPointForPosition(false);
    consoleTab->setID("tab-container-console"_spr);

    m_mainLayer->addChildAtPosition(scriptTab, Anchor::Center);
    m_mainLayer->addChildAtPosition(consoleTab, Anchor::Center);

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
            "AdwaitaMono-Regular.fnt"_spr,
            0.75f * Settings::consoleFontSize());
        consoleText->setAnchorPoint({0, 0});
        consoleText->ignoreAnchorPointForPosition(false);
        consoleText->setContentSize(consoleOutSize - CCSize(20, 20));
        consoleText->setID("console-output-text"_spr);

        consoleWindow->addInnerChild(consoleText);
        consoleTab->addChildAtPosition(consoleWindow, Anchor::Center, consoleOutOffset);

        consoleInput = TextInput::create(consoleInSize.width - 40, "", "AdwaitaMono-Regular.fnt"_spr);
        //i thought this was a hacky way to make the text in the text field left aligned
        //but then i realized this is literally what the one on the level search page does
        consoleInput->getInputNode()->setAnchorPoint({0.5, 0});
        consoleInput->getInputNode()->ignoreAnchorPointForPosition(false);
        consoleInput->getInputNode()->m_textField->setAnchorPoint({0, 0.5});
        consoleInput->getInputNode()->m_textLabel->setAnchorPoint({0, 0.5});
        consoleInput->setID("console-input"_spr);

        consoleSendButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(">", 30, false, "bigFont.fnt", "GJ_button_01.png", 30, 1.0f),
            this, menu_selector(ScriptSelectorPopup::sendConsoleInput));
        consoleSendButton->setID("console-send-button"_spr);

        /*this->addEventListener<InvokeBindFilterV2>([this](InvokeBindEventV2* event) {
            if (event->isDown() && consoleTab->isVisible() && consoleInput->getInputNode()->m_selected) {
                sendConsoleInput(nullptr);
                return ListenerResult::Stop;
            }

            return ListenerResult::Propagate;
        }, "lua-console-send"_spr);*/

        consoleTab->addChildAtPosition(consoleSendButton, Anchor::Center, consoleInOffset + CCPoint((consoleInSize.width - 40) / 2.0f + 3, 0));
        consoleTab->addChildAtPosition(consoleInput, Anchor::Center, consoleInOffset - CCPoint(20, 0));
    }

    loadingIndicator = LoadingCircleSprite::create(1.0f);
    loadingIndicator->setScale(0.45f);

    auto cancelButtonSprite = CCSprite::createWithSpriteFrameName("GJ_stopEditorBtn_001.png");
    if (!cancelButtonSprite) {
        log::error("Cancel button sprite not present");
        return false;
    }
    cancelButtonSprite->setScale(0.7f);
    cancelButton = CCMenuItemSpriteExtra::create(cancelButtonSprite, this, menu_selector(ScriptSelectorPopup::onClickCancel));

    statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
    statusLabel->setAnchorPoint({0, 0.5});

    updateScriptStatus();

    m_mainLayer->addChildAtPosition(loadingIndicator, Anchor::BottomLeft, {55, 22.5});
    m_buttonMenu->addChildAtPosition(cancelButton, Anchor::BottomLeft, {55, 22.5});
    m_mainLayer->addChildAtPosition(statusLabel, Anchor::BottomLeft, {85, 22.5});

    //scheduleUpdate();
    return true;
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
    /*auto lineLabel = CCLabelBMFont::create(line.c_str(),
        "chatFont.fnt",
        (consoleBG->getContentWidth() - 20) / 0.75f,
        kCCTextAlignmentLeft);
    lineLabel->setScale(0.75f);
    lineLabel->setAnchorPoint({0, 0});
    lineLabel->ignoreAnchorPointForPosition(false);
    lineLabel->setPosition(0, 0);*/

    float logHeight = (consoleText->lineCount() + 1) * consoleText->getLineHeight();
    //log::info("logheight is {}", logHeight);
    //log::info("innerheight is {}", consoleWindow->innerHeight());
    if (logHeight > consoleWindow->innerHeight()) {
        //log::info("will grow");
        float newHeight = logHeight;
        consoleWindow->resizeInner(newHeight);
        consoleText->setContentHeight(newHeight);
    }

    /*CCArrayExt<CCLabelBMFont*> labels = consoleWindow->getInnerChildren();
    for (auto label : labels) {
        label->setPositionY(label->getPositionY() + consoleLineHeight);
    }

    consoleWindow->addInnerChild(lineLabel);*/
    consoleText->addLine(line);
}
void ScriptSelectorPopup::sendConsoleInput(CCObject*) {
    auto str = consoleInput->getString();
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
            scriptTab->setVisible(true);
            consoleTab->setVisible(false);
            break;
        case 1:
            scriptTab->setVisible(false);
            consoleTab->setVisible(true);
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
    auto result = LuaEngine::get()->executeAsync(editor, data.value(), script.name);
    updateScriptStatus();
    result.listen([name = script.name, engine = engine](ScriptResult* resultP) {
        //log::info("script {} finished with result {}", name, static_cast<int>(resultP->result));
        engine->updateScriptStatusForConsole();
        auto result = *resultP;
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
    }, [engine = engine](const ScriptExecutionStatus** status) {
        engine->updateScriptStatusForConsole();
    });
}

void ScriptSelectorPopup::onClickCancel(CCObject* object) {
    auto status = engine->getStatus();
    if (status.type != STOPPED && status.asyncData.isAsync)
        status.asyncData.task.cancel();
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


ScriptSelectorPopup* ScriptSelectorPopup::create(EditorUI* editor) {
    auto ret = new ScriptSelectorPopup();
    if (ret->initAnchored(380.0f, 280.0f, editor)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}