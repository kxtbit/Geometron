// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppCStyleCast
// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppHidingFunction
#include "LuaUtils.hpp"

#include <generator>
#include <string>

#include <Geode/Geode.hpp>

#include "LuaEngine.hpp"
#include "../utils/types/GPoint.hpp"
#include "../utils/FontUtils.hpp"
#include "../hooks/TrackedGameObject.hpp"

using namespace geode::prelude;
using namespace std::string_literals;

bool gameObjectExists(GameObject* object) {
    return static_cast<TrackedGameObject*>(object)->m_fields->objectExists;
}
sol::object wrapGameObject(lua_State* L, GameObject* object) {
    return sol::make_object(L, std::move(Ref(object)));
}
sol::table wrapArrayOfGameObjects(lua_State* L, CCArray* objects) {
    auto output = sol::table::create(L, objects->count());

    for (int i = 0; i < objects->count(); i++) {
        output.set(i + 1, wrapGameObject(L, static_cast<GameObject*>(objects->objectAtIndex(i))));
    }
    return output;
}

void updateObjectPosition(EditorUI* editor, GameObject* object) {
    if (!gameObjectExists(object)) return;
    auto layer = editor->m_editorLayer;

    //object->quickUpdatePosition();
    object->updateStartValues();
    layer->removeObjectFromSection(object);
    layer->addToSection(object);

    bool speedObjectsUpdated = false;
    switch (object->m_objectID) {
        default: break;
        case 899: //color trigger, + legacy color triggers
        case 29: //BG
        case 30: //GRND
        case 105: //OBJ
        case 744: //3DL
        case 900: //GRND 2
        case 915: //LINE
            layer->m_colorTriggersChanged = true;
            break;
        case 1007: //alpha trigger
            layer->m_alphaTriggersChanged = true;
            break;
        case 1006: //pulse trigger
            layer->m_pulseTriggersChanged = true;
            break;
        case 747: //old teleport portal
            if (auto casted = typeinfo_cast<TeleportPortalObject*>(object)) {
                updateObjectPosition(editor, casted->m_orangePortal);
            }
            break;
            //i think this section is for various objects that change the flow of gameplay?
        case 200: //slow speed portal
        case 201: //1x speed portal
        case 202: //2x speed portal
        case 203: //3x speed portal
        case 1334: //4x speed portal
        case 1917: //reverse trigger
        case 1935: //timewarp trigger
        case 2900: //rotate gameplay trigger / arrow trigger
        case 2902: //new teleport portal
        case 3022: //teleport trigger
        case 3027: //teleport orb
            speedObjectsUpdated = true;
            break;
            //there is supposed to be something for the legacy enter effects but i can't understand the decompilation :(
    }
    if (object->m_isSpawnOrderTrigger)
        layer->m_spawnOrderObjectsChanged = true;
    if (object->m_dontIgnoreDuration) if (auto casted = typeinfo_cast<EffectGameObject*>(object))
        casted->m_endPosition = CCPoint {0, 0};
    if (speedObjectsUpdated || object->canReverse())
        editor->m_speedObjectsUpdated = true;
}
void updateEditorSelection(EditorUI* editor) {
    editor->updateObjectInfoLabel();
}
void purgeObjectFromUndoHistory(EditorUI* editor, CCArray* history, GameObject* object) {
    for (int i = history->count() - 1; i >= 0; i--) {
        auto undo = static_cast<UndoObject*>(history->objectAtIndex(i));

        if (undo->m_objects != nullptr) {
            if (undo->m_objects->containsObject(object)) {
                //log::info("object in array, purge undo");
                goto purge;
            }
        } else if (undo->m_objectCopy != nullptr && undo->m_objectCopy->m_object == object) {
            //log::info("object match, purge undo");
            purge:
            //remove all entries later in the history too
            //because they might depend on the existence of this entry
            for (int j = i; j >= 0; j--)
                history->removeObjectAtIndex(j);
            editor->updateButtons();
            break;
        }
    }
}
void prepareToDeleteObject(EditorUI* editor, GameObject* object) {
    auto editorLayer = editor->m_editorLayer;
    if (object->m_isSelected) {
        if (editor->m_selectedObject == object) {
            editor->deselectAll();
        } else {
            editor->deselectObject(object);
        }
    }

    //a deleted object is not valid in the editor
    //if the user tries to undo or redo an operation on that object it might crash
    //so instead just remove the undo and redo entries up to and including the object
    purgeObjectFromUndoHistory(editor, editorLayer->m_undoObjects, object);
    purgeObjectFromUndoHistory(editor, editorLayer->m_redoObjects, object);
}
GameObject* safeCreateObject(EditorUI* editor, int id, GPoint pos) {
    constexpr int maxObjectID = 4539; //as of 2.2071

    if (id < 1 || id > maxObjectID) return nullptr;
    //cannot create the linked orange teleport portal by itself
    if (id == 749) return nullptr;

    auto editorLayer = editor->m_editorLayer;
    //last parameter true i think makes it not count in the undo history
    auto object = editorLayer->createObject(id, CCPoint(pos + GPoint {0, 90}), true);
    //make sure editor layer is 0 instead of whatever the editor is currently set to
    object->m_editorLayer = 0;
    switch (id) {
        case 914: //text object
            static_cast<TextGameObject*>(object)->updateTextObject("a"s, false);
            break;
        case 3032: //keyframe object
            //set the keyframe group to -1 so the editor won't try to add it to any existing keyframe anims
            static_cast<KeyframeGameObject*>(object)->m_keyframeGroup = -1;
            break;
        default: break;
    }
    if (object->m_classType == GameObjectClassType::Effect) {
        auto effect = static_cast<EffectGameObject*>(object);
        if (effect->isSpeedObject() || !effect->canReverse() || !effect->m_isReverse) {
            effect->m_shouldPreview = true;
            auto speedObjects = editorLayer->m_drawGridLayer->m_speedObjects;
            if (!speedObjects->containsObject(effect)) {
                speedObjects->addObject(effect);
                editorLayer->m_drawGridLayer->m_updateSpeedObjects = true;
                effect->updateSpeedModType();
            }
        }
    }

    return object;
}
void safeDeleteObject(EditorUI* editor, GameObject* object) {
    switch (object->m_objectID) {
        case 747:
            //make sure nothing holds a reference to the orange portal
            prepareToDeleteObject(editor, static_cast<TeleportPortalObject*>(object)->m_orangePortal);
            break;
        case 749:
            //cannot delete the linked orange portal by itself
            return;
        default: break;
    }

    prepareToDeleteObject(editor, object);

    editor->deleteObject(object, true); //true so it doesn't create an undo command for each one
}
GPoint getEditorViewCenter(EditorUI* editor) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto screenCenter = GPoint {winSize.width / 2, winSize.height / 2 + editor->m_toolbarHeight / 2};
    return GPoint(editor->m_editorLayer->m_objectLayer->convertToNodeSpace(CCPoint(screenCenter))) + GPoint {0, -90};
}
void setEditorViewCenter(EditorUI* editor, GPoint pos) {
    auto objectLayer = editor->m_editorLayer->m_objectLayer;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto screenCenter = GPoint {winSize.width / 2, winSize.height / 2 + editor->m_toolbarHeight / 2};

    auto layerPos = (pos + GPoint {0, 90}) * -objectLayer->getScale();
    auto layerOriginPos = layerPos + screenCenter;
    objectLayer->setPosition(CCPoint(layerOriginPos));
}
static int loadCurrentFont(GameManager* gameManager) {
    constexpr int maxFont = 59; //as of 2.2074

    auto loadedFont = std::clamp(gameManager->m_loadedFont, 0, maxFont);

    gameManager->loadFont(loadedFont);
    return loadedFont;
}
static std::string fontIDToString(int fontID) {
    if (fontID == 0) {
        return "bigFont.fnt"s;
    } else {
        return std::move(fmt::format("gjFont{:02d}.fnt", fontID));
    }
}
#include <Geode/modify/GameManager.hpp>
class $modify(GameManagerFontData, GameManager) {
    struct Fields {
        Ref<CCBMFontConfiguration> fontData;
    };

    $override
    void loadFont(int index) {
        GameManager::loadFont(index);

        auto fontName = fontIDToString(m_loadedFont);
        m_fields->fontData = CCBMFontConfiguration::create(fontName.c_str());
    }
};
std::string getCurrentFontName() {
    auto gameManager = GameManager::get();
    return fontIDToString(loadCurrentFont(gameManager));
}
CCBMFontConfiguration* getCurrentFontConfiguration() {
    auto gameManager = GameManager::get();
    loadCurrentFont(gameManager);
    return static_cast<GameManagerFontData*>(gameManager)->m_fields->fontData;
}
CCArrayExt<TextGameObject> splitText(EditorUI* editor, TextGameObject* text) {
    auto editorLayer = editor->m_editorLayer;

    auto fontData = getCurrentFontConfiguration();

    CCArrayExt<CCFontSprite> sprites = text->getChildren();
    CCArrayExt<TextGameObject> objects = CCArray::createWithCapacity(sprites.size());
    auto str = text->m_text;
    auto ccwstr = cc_utf8_to_utf16(str.c_str());
    utf16string wstr = ccwstr;
    CC_SAFE_DELETE_ARRAY(ccwstr);

    for (int i = 0, spriteIndex = 0; i < wstr.size(); i++) {
        utf16char c = wstr[i];
        if (c == ' ') {
            spriteIndex++;
            continue;
        }
        if (c == '\n' || !fontData->m_pCharacterSet->contains(c)) continue;

        auto sprite = sprites[spriteIndex];
        auto localPos = sprite->getPosition();
        auto worldPos = CCPointApplyAffineTransform(localPos, text->nodeToParentTransform());
        auto newText = static_cast<TextGameObject*>(editorLayer->createObject(914, worldPos, true));
        newText->updateTextObject(str.substr(i, 1), false);
        newText->updateCustomScaleX(text->m_scaleX);
        newText->updateCustomScaleY(text->m_scaleY);
        newText->setRotationX(text->getRotationX());
        newText->setRotationY(text->getRotationY());
        auto newSprite = newText->getChildByType<CCFontSprite>(0);
        {
            auto spritePos = newSprite->getPosition();
            auto screenSpritePos = newText->convertToWorldSpace(spritePos);
        }
        //no convertToNodeSpace here
        //because newText technically does not have a parent yet
        auto offset = newText->getPosition() -
            newText->convertToWorldSpace(newSprite->getPosition());
        editorLayer->removeObjectFromSection(newText);
        newText->setPosition(newText->getPosition() + offset);
        editorLayer->addToSection(newText);
        objects.push_back(newText);

        spriteIndex++;
    }

    return std::move(objects);
}

void addProtectedMT(sol::state_view& lua, std::string_view name) {
    lua_State* L = lua.lua_state();

    //log::info("protect MT {}", name);
    lua_pushlstring(L, name.data(), name.size());
    lua_rawget(L, LUA_REGISTRYINDEX);
    pushConstantString<"__metatable">(L);
    lua_pushlstring(L, protectedMTMessage.data(), protectedMTMessage.size());
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

template<class F>
struct YieldingFunctionData {
    sol::variadic_args args;
    int startOfParams;
    YieldingFunctionCoroutine co;
    F function;
};
template<class F>
static int yieldingFunctionContinuation(lua_State* L, int status, lua_KContext context) {
    auto* data = static_cast<YieldingFunctionData<F>*>(lua_touserdata(L, lua_upvalueindex(1)));

    data->args = sol::variadic_args(L, data->startOfParams, lua_gettop(L));

    data->co.resume();
    auto& promise = data->co.promise();
    int numReturns = promise.stackEnd - promise.stackStart;
    data->startOfParams = promise.stackStart;
    if (data->co.done()) {
        //log::info("returning yielding function with {} values", numReturns);
        return numReturns;
    } else {
        //log::info("yielding from yielding function with {} values", numReturns);
        return lua_yieldk(L, numReturns, 0, &yieldingFunctionContinuation<F>);
    }
}
template<class F>
static int yieldingFunctionStub(lua_State* L) {
    auto* data = static_cast<YieldingFunctionData<F>*>(lua_touserdata(L, lua_upvalueindex(1)));

    data->args = sol::variadic_args(L, 1, lua_gettop(L));
    data->startOfParams = 1;
    data->co = data->function((sol::variadic_args&) data->args, L);

    data->co.resume();
    auto& promise = data->co.promise();
    int numReturns = promise.stackEnd - promise.stackStart;
    data->startOfParams = promise.stackStart;
    if (data->co.done()) {
        //log::info("returning yielding function with {} values", numReturns);
        return numReturns;
    } else {
        //log::info("yielding from yielding function with {} values", numReturns);
        return lua_yieldk(L, numReturns, 0, &yieldingFunctionContinuation<F>);
    }
}

template<class F>
sol::function yieldingFunction(sol::state_view& lua, F func) {
    auto L = lua.lua_state();
    lua_checkstack(L, 2);
    auto data = static_cast<YieldingFunctionData<F>*>(lua_newuserdatauv(L, sizeof(YieldingFunctionData<F>), 0));
    lua_pushcclosure(L, &yieldingFunctionStub<F>, 1);
    data->function = func;

    return sol::stack::get<sol::function>(L, lua_absindex(L, -1));
}

//template int yieldingFunctionContinuation<YieldingFunctionCoroutine (*)(sol::variadic_args, curengine)>(lua_State*, int, lua_KContext);
template sol::function yieldingFunction(sol::state_view& lua, YieldingFunctionCoroutine (*)(sol::variadic_args, lua_State*));