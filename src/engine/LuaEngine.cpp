// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
#include <Geode/Geode.hpp>

#include "LuaEngine.hpp"

#include "../../external/lua/upstream/lstate.h"
#include "../utils/Settings.hpp"

using namespace std::string_literals;
using namespace geode::prelude;

LuaEngine* LuaEngine::instance;

static inline void luaPanic(sol::optional<std::string> msg) {
    if (msg) {
        log::error("Lua panic triggered: {}", msg.value());
    } else {
        log::error("Lua panic triggered with no error info");
    }
    //something went horribly wrong, manually trigger a crash so at least Lua doesn't abort() and we can get some error info
    *reinterpret_cast<volatile int*>(0) = 1;
}
void LuaEngine::init() {
    log::info("Initializing Lua engine...");
    if (!tryLock())
        log::warn("The Lua engine is already locked before initialization, things might break!");

    stateSetup();
    consoleLines.push_back("");

    class Updater : public CCObject {
    public:
        LuaEngine* engine;
        explicit Updater(LuaEngine* newEngine) {
            engine = newEngine;
        }

        void update(float dt) override {
            engine->update(dt);
        }
    };

    updater = new Updater(this);
    CCScheduler::get()->scheduleUpdateForTarget(updater, 0, false);

    unlock();
}

template<int (*Continuation)(lua_State*, int, lua_KContext)>
static int doPostResume(lua_State* L, lua_State* co, int status, int nCoRets) {
    //based on luaB_coresume from lcorolib.c
    LuaEngine* engine = LuaEngine::get(L);
    if (status == LUA_YIELD && engine->executionStatus.type == YIELDING) {
        //YIELDING means the whole Lua engine is supposed to be suspended
        //so we yield again to bubble the yield up to the main thread
        if (nCoRets > 0)
            luaL_error(L, "invalid engine yield, should have no parameters");
        return lua_yieldk(L, 0, reinterpret_cast<lua_KContext>(co), Continuation);
    } else if (status == LUA_OK || status == LUA_YIELD) {
        if (!lua_checkstack(L, nCoRets + 1)) {
            lua_pop(co, nCoRets); //not enough space to move the results, so just destroy them
            lua_pushboolean(L, false);
            lua_pushliteral(L, "too many results to resume");
            return 2;
        }
        lua_pushboolean(L, true);
        lua_xmove(co, L, nCoRets); //move the results
        return nCoRets + 1;
    } else {
        lua_pushboolean(L, false);
        lua_xmove(co, L, 1); //move the error message
        return 2;
    }
}
template<int (*ReturnHandler)(lua_State*, int)>
static int coroutineResumeContinuation(lua_State* L, int, lua_KContext ctx) {
    //believe me i would love not to reinterpret_cast here but we are in C land :/
    lua_State* co = reinterpret_cast<lua_State*>(ctx);

    if (lua_gettop(L) > 1) {
        return luaL_error(L, "invalid engine resume, should have no parameters");
    }

    int nCoRets;
    int status = lua_resume(co, L, 0, &nCoRets);
    return ReturnHandler(L, doPostResume<&coroutineResumeContinuation<ReturnHandler>>(L, co, status, nCoRets));
}
static int customCoroutineResume(lua_State* L) {
    //based on luaB_coresume from lcorolib.c
    int nArgs = lua_gettop(L);

    lua_State* co = lua_tothread(L, 1);
    luaL_argexpected(L, co, 1, "thread");

    int nCoArgs = nArgs - 1;
    if (!lua_checkstack(co, nCoArgs)) {
        lua_pushboolean(L, false);
        lua_pushliteral(L, "too many arguments to resume");
        return 2;
    }
    lua_xmove(L, co, nCoArgs); //move arguments to coroutine
    int nCoRets;
    int status = lua_resume(co, L, nCoArgs, &nCoRets);
    //having a lambda as a template parameter is so cursed
    return doPostResume<&coroutineResumeContinuation<[](lua_State*, int r) { return r; }>>(L, co, status, nCoRets);
}
static int coroutineWrapperReturn(lua_State* L, int nPostResumeRets) {
    bool success = lua_toboolean(L, -nPostResumeRets);
    if (success) {
        return nPostResumeRets - 1;
    } else {
        if (nPostResumeRets != 2)
            return luaL_error(L, "solar bitflip is uncool >:(");
        return lua_error(L);
    }
}
static int coroutineWrapper(lua_State* L) {
    int nArgs = lua_gettop(L);

    lua_State* co = lua_tothread(L, lua_upvalueindex(1));

    if (!lua_checkstack(co, nArgs)) {
        return luaL_error(L, "too many arguments to resume");
    }
    lua_xmove(L, co, nArgs); //move arguments to coroutine
    int nCoRets;
    int status = lua_resume(co, L, nArgs, &nCoRets);
    return doPostResume<&coroutineResumeContinuation<&coroutineWrapperReturn>>(L, co, status, nCoRets);
}
static int customCoroutineWrap(lua_State* L) {
    //based on luaB_cowrap from lcorolib.c
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_State* co = lua_newthread(L);
    lua_pushvalue(L, 1);
    //in Lua C api, coroutines ("threads") are not technically bound to functions
    //instead the function must be pushed as the first element of the stack for the new coroutine
    lua_xmove(L, co, 1);
    lua_pushcclosure(L, &coroutineWrapper, 1);
    return 1;
}
void LuaEngine::stateSetup() {
    //lua.~state();
    lua = nullptr;
    lua = sol::state(sol::c_call<decltype(&luaPanic), &luaPanic>);
    //debugTrap();
    editor = nullptr;

    *static_cast<LuaEngine**>(lua_getextraspace(lua.lua_state())) = this;

    lua.open_libraries(sol::lib::base,
        sol::lib::math,
        sol::lib::string,
        sol::lib::table,
        sol::lib::utf8,
        sol::lib::coroutine,
        sol::lib::os,
        sol::lib::debug);
    luaAddIO(lua);

    lua["loadfile"] = sol::nil;
    lua["dofile"] = sol::nil;

    lua["os"]["execute"] = sol::nil;
    lua["os"]["exit"] = sol::nil;
    lua["os"]["getenv"] = sol::nil;
    lua["os"]["remove"] = sol::nil;
    lua["os"]["rename"] = sol::nil;
    lua["os"]["tmpname"] = sol::nil;

    lua["coroutine"]["resume"] = sol::as_function(&customCoroutineResume);
    lua["coroutine"]["wrap"] = sol::as_function(&customCoroutineWrap);

    auto engineTable = lua.create_table(0, 1);
    engineTable.set_function("sleep", [](lua_State* L) -> int {
        auto duration = luaL_checknumber(L, 1);
        auto engine = LuaEngine::get(L);
        if (!engine->isAsynchronous())
            return luaL_error(L, "script is not running in asynchronous mode, will not block main thread");
        engine->executionStatus.type = YIELDING;
        engine->executionStatus.yieldType = SLEEPING;
        engine->executionStatus.yieldData.remainingSleepDuration = duration;
        return lua_yield(L, 0);
    });

    lua["engine"] = engineTable;
}
void LuaEngine::editorSetup(EditorUI* editor) {
    //log::info("begin editor setup");
    this->editor = editor;

    log::debug("adding enums");
    luaAddEnums(lua);
    log::debug("adding usertypes");
    luaAddUsertypes(lua, editor);

    log::debug("adding editor API");
    sol::table editorTable = lua.create_table("editor");
    editorTable.set_function("getSelectedObjects", [](curengine engine, sol::this_state lua) {
        sol::state_view state(lua);
        return wrapArrayOfGameObjects(state, engine->editor->getSelectedObjects());
    });
    editorTable.set_function("getAllObjects", [](curengine engine, sol::this_state lua) {
        sol::state_view state(lua);
        return wrapArrayOfGameObjects(state, engine->editor->m_editorLayer->m_objects);
    });
    editorTable.set_function("createObject", [](int id, curengine engine) {
        auto editorLayer = engine->editor->m_editorLayer;
        auto object = GameObject::createWithKey(id);//engine->editor->createObject(id, {0, 90});
        if (id == 3032) { //keyframe object
            //set the keyframe group to -1 so the editor won't try to add it to any existing keyframe anims
            static_cast<KeyframeGameObject*>(object)->m_keyframeGroup = -1;
        }
        object->setVisible(false);
        object->customSetup();
        object->saveActiveColors();
        editorLayer->addToSection(object);
        editorLayer->addSpecial(object);
        return wrapGameObject(engine->lua, object);
    });
    editorTable.set_function("loadObjects", [](std::string data, curengine engine, sol::this_state lua) {
        sol::state_view state(lua);
        return wrapArrayOfGameObjects(state, engine->editor->m_editorLayer->createObjectsFromString(data, true, true));
    });
    editorTable.set_function("removeObjects", [](sol::variadic_args args, curengine engine) {
        auto editorLayer = engine->editor->m_editorLayer;
        for (sol::optional<GameObject*> object : args) {
            if (!object.has_value()) continue;
            editorLayer->removeObject(object.value(), true); //true so it doesn't create an undo command for each one
        }
    });
    log::debug("added editor properties 1");

    editorTable.set_function("removeParentGroups", [](sol::variadic_args args, curengine engine) {
        auto editorLayer = engine->editor->m_editorLayer;
        for (sol::optional<int> group : args) {
            if (!group.has_value()) continue;
            editorLayer->removeGroupParent(group.value());
        }
    });
    editorTable.set_function("linkObjects", [](sol::variadic_args args, curengine engine) {
        CCArrayExt<GameObject*> array = CCArray::createWithCapacity(args.size());
        for (sol::optional<GameObject*> object : args) {
            if (!object.has_value()) continue;
            array.push_back(object.value());
        }
        engine->editor->m_editorLayer->groupStickyObjects(array.inner());
    });
    editorTable.set_function("unlinkObjects", [](sol::variadic_args args, curengine engine) {
        CCArrayExt<GameObject*> array = CCArray::createWithCapacity(args.size());
        for (sol::optional<GameObject*> object : args) {
            if (!object.has_value()) continue;
            array.push_back(object.value());
        }
        engine->editor->m_editorLayer->ungroupStickyObjects(array.inner());
    });
    log::debug("added editor properties 2");

    editorTable.set_function("linkKeyframes", [](sol::variadic_args args, curengine engine) {
        auto editorLayer = engine->editor->m_editorLayer;
        int keyframeGroup = editorLayer->m_keyframeGroup++;
        int keyframeIndex = 0;
        CCArrayExt<GameObject*> keyframes = CCArray::createWithCapacity(args.size());
        for (sol::optional<GameObject*> object : args) {
            if (!object.has_value()) continue;
            auto keyframe = typeinfo_cast<KeyframeGameObject*>(object.value());
            if (!keyframe) continue;
            keyframe->m_keyframeGroup = keyframeGroup;
            keyframe->m_keyframeIndex = keyframeIndex++;
            keyframes.push_back(keyframe);
        }
        editorLayer->m_keyframeGroups->setObject(keyframes.inner(), keyframeGroup);
    });
    editorTable.set_function("unlinkKeyframeGroup", [](int keyframeGroup, curengine engine) {
        auto editorLayer = engine->editor->m_editorLayer;
        auto keyframes = static_cast<CCArray*>(editorLayer->m_keyframeGroups->objectForKey(keyframeGroup));
        if (!keyframes) return;
        for (int i = 0; i < keyframes->count(); i++) {
            auto keyframe = static_cast<KeyframeGameObject*>(keyframes->objectAtIndex(i));
            keyframe->m_keyframeGroup = -1;
        }
        editorLayer->m_keyframeGroups->removeObjectForKey(keyframeGroup);
    });
    log::debug("added editor properties 3");

    editorTable.set_function("nextFreeColorChannel", [](curengine engine) {
        return engine->editor->m_editorLayer->getNextColorChannel();
    });
    editorTable.set_function("nextFreeGroupID", [](curengine engine) {
        return engine->editor->m_editorLayer->getNextFreeGroupID(nullptr);
    });
    editorTable.set_function("nextFreeItemID", [](curengine engine) {
        return engine->editor->m_editorLayer->getNextFreeItemID(nullptr);
    });
    editorTable.set_function("nextFreeGradientID", [](curengine engine) {
        return engine->editor->m_editorLayer->getNextFreeGradientID(nullptr);
    });
    editorTable.set_function("nextFreeAreaEffectID", [](curengine engine) {
        return engine->editor->m_editorLayer->getNextFreeAreaEffectID(nullptr);
    });

    editorTable.set_function("getObjectRect", [](GameObject* object, curengine engine) {
        //according to decompilation the last two parameters do nothing
        auto rect = engine->editor->m_editorLayer->getObjectRect(object, false, false);
        auto p1 = LuaPoint {rect.getMinX(), rect.getMinY()};
        auto p2 = LuaPoint {rect.getMaxX(), rect.getMaxY()};
        return std::tuple {p1, p2};
    });
    log::debug("added editor properties 4");

    //sol::table gd = lua.create_table("gd");

    lua["editor"] = editorTable;

    //log::info("end editor setup");
}

static std::string lineTrim(const std::string& line) {
    return line.ends_with('\n') ? line.substr(0, line.size() - 1) : line;
}

void LuaEngine::sendToConsole(const std::string& text) {
    if (text.size() <= 0) return;
    if (text == "\n") {
        if (consoleLines.size() < 1 || consoleLines[consoleLines.size() - 1].ends_with('\n')) {
            consoleLines.push_back("\n");
            if (currentConsole) currentConsole->onConsoleLine(" ");
        } else {
            consoleLines[consoleLines.size() - 1].append("\n");
        }
        return;
    }

    std::vector<std::string> lines;
    int i = 0;
    while (i < text.size()) {
        for (int j = i; j < text.size(); j++) {
            if (text.at(j) == '\n') {
                lines.push_back(text.substr(i, j - i + 1));
                i = j + 1;
                goto next;
            }
        }
        lines.push_back(text.substr(i));
        break;
        next:
        continue;
    }
    //for (auto line : lines) {
    //    log::info("console write line {}", escapeString(line));
    //}
    //auto lastLine = lines[lines.size() - 1];
    //if (lastLine == "\n" || lastLine.size() <= 0) lines.erase(lines.end() - 1);

    if (consoleLines.size() < 1 || consoleLines[consoleLines.size() - 1].ends_with('\n')) {
        consoleLines.push_back(lines[0]);
        if (currentConsole) currentConsole->onConsoleLine(lineTrim(lines[0]));
    } else {
        consoleLines[consoleLines.size() - 1].append(lines[0]);
        if (currentConsole) currentConsole->onConsoleText(lineTrim(lines[0]));
    }
    for (auto l = lines.begin() + 1; l < lines.end(); l += 1) {
        consoleLines.push_back(*l);
        if (currentConsole) currentConsole->onConsoleLine(lineTrim(*l));
    }
}

static void luaStackDump(lua_State* L) {
    log::debug("stack dump:");
    for (int i = -lua_gettop(L); i < 0; i++) {
        if (lua_isfunction(L, i)) {
            lua_Debug d;
            lua_pushvalue(L, i);
            lua_getinfo(L, ">nS", &d);
            log::debug("{}: {} from {}", i, d.name ? d.name : "unknown", d.source ? d.source : "unknown");
        } else {
            size_t len;
            auto str = luaL_tolstring(L, i, &len);
            log::debug("{}: {}", i, std::string(str, len));
            lua_pop(L, 1);
        }
    }
    luaL_traceback(L, L, nullptr, 0);
    size_t len;
    auto tb = lua_tolstring(L, -1, &len);
    log::debug("{}", std::string(tb, len));
    lua_pop(L, 1);
}

static void handleYield(LuaEngine* self, int numRets) {
    log::debug("yielded with {} values", numRets);
    lua_State* L = self->lua.lua_state();
    //lua_pop(L, numRets);
    if (self->executionStatus.type == YIELDING) {
        if (numRets > 0)
            log::warn("Lua state yielded to engine with more than zero values");
    } else {
        //for whatever reason the Lua script called coroutine.yield() at the top level
        self->executionStatus.type = YIELDING;
        self->executionStatus.yieldType = MANUAL;
    }
    self->updateExecutionStatus();
}
static void luaPreemptionHook(lua_State* L, lua_Debug* debug) {
    {
        log::debug("hit preemption hook");
        auto engine = LuaEngine::get(L);

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto timeSinceResume = currentTime - engine->executionStatus.resumeTime;
        float msSinceResume = std::chrono::duration_cast<std::chrono::microseconds>(timeSinceResume).count() / 1000.0;

        float maxTime = Settings::luaMaxExecutionTime();
        if (msSinceResume >= maxTime) {
            if (lua_isyieldable(L)) {
                engine->executionStatus.type = YIELDING;
                engine->executionStatus.yieldType = PREEMPTED;
                engine->updateExecutionStatus();
                log::debug("preempting script");
                goto yield;
            } else if (float graceTime = Settings::luaUninterruptibleGraceTime(); msSinceResume >= graceTime) {
                //the script can't yield (maybe it is in a metamethod) and exhausted its grace time of uninterruptible execution
                //so instead just repeatedly throw errors to try to terminate the script as fast as possible
                log::debug("terminating script");
                engine->executionStatus.type = TERMINATING_TIMEOUT;
                engine->updateExecutionStatus();
                lua_sethook(L, &luaPreemptionHook, LUA_MASKCOUNT, 1); //run the hook every 1 instructions
                goto kill;
            }
        }
        return;
    }
    //make sure all the C++ stuff is deallocated when we yield or error
    yield:
    lua_yield(L, 0);
    return;
    kill:
    //this is super hacky but it's the only way to make sure the hook doesn't call into Lua code
    //if there is an error handler present (e.g. from xpcall) lua_error will call it first
    //since any code called within a debug hook has debug hooks disabled, the Lua script escapes termination
    L->errfunc = 0;
    lua_pushstring(L, "script has been uninterruptible for too long");
    lua_error(L);
    return;
}
static void updatePreemption(LuaEngine* self) {
    lua_sethook(self->lua.lua_state(), &luaPreemptionHook, LUA_MASKCOUNT, Settings::luaInterruptResolution());
    self->executionStatus.resumeTime = std::chrono::high_resolution_clock::now();
}
static void preRunSetup(LuaEngine* self) {
    self->consoleLines.clear();
}

void LuaEngine::onLuaWrite(const std::string& data) {
    //log::debug("output sent: {}", data);
    sendToConsole(data);
}

static bool isTerminating(ScriptExecutionStatus status) {
    return status.type == TERMINATING_TIMEOUT || status.type == TERMINATING_CANCELED;
}
static std::string terminationMessage(ScriptExecutionType type) {
    switch (type) {
        case TERMINATING_TIMEOUT:
            return "Script reached its limit of uninterruptible execution time"s;
        case TERMINATING_CANCELED:
            return "Script execution was canceled"s;
        default: return "(invalid termination message)";
    }
}

void LuaEngine::updateExecutionStatus() const {
    if (executionStatus.asyncData.isAsync && executionStatus.asyncData.postProgress)
        executionStatus.asyncData.postProgress(&executionStatus);
}

void LuaEngine::update(float dt) {
    if (executionStatus.type == YIELDING && executionStatus.yieldType == SLEEPING)
        executionStatus.yieldData.remainingSleepDuration -= dt;
    tryResume();
}
void LuaEngine::tryResume() {
    //if there is definitely no script running, don't bother trying to get the lock
    if (!fastIsExecuting) return;
    if (!tryLock()) return;

    if (executionStatus.type == YIELDING) {
        //log::info("try resume script, yield type is {}", static_cast<int>(executionStatus.yieldType));
        lua_State* L = lua.lua_state();
        if (executionStatus.asyncData.isAsync && executionStatus.asyncData.isCancelled && executionStatus.asyncData.isCancelled()) {
            executionStatus.asyncData.postResult({SCRIPT_CANCELED});

            lua_settop(L, 0);
            executionStatus.type = STOPPED;
            executionStatus.asyncData.isAsync = false;
            updateExecutionStatus();
            fastIsExecuting = false;

            updateScriptStatusForConsole();

            stateSetup();
        }
        bool shouldResume = true;
        switch (executionStatus.yieldType) {
            case SLEEPING:
                shouldResume = executionStatus.yieldData.remainingSleepDuration <= 0;
                break;
            case WAITING_FOR_CONSOLE_NEWLINE:
                shouldResume = false;
                if (luaInBufferMutex.try_lock()) {
                    shouldResume = luaInBufferNewlineWritten;
                    luaInBufferMutex.unlock();
                }
                break;
            case WAITING_FOR_CONSOLE_CHARACTERS:
                shouldResume = false;
                if (luaInBufferMutex.try_lock()) {
                    shouldResume = luaInBufferPendingCharacters == 0;
                    luaInBufferMutex.unlock();
                }
                break;
            case PREEMPTED:
            case MANUAL:
                shouldResume = true;
                break;
        }
        if (shouldResume) {
            log::debug("shouldResume is {}", shouldResume);

            int nRets;
            executionStatus.type = RUNNING;
            fastIsExecuting = true;
            updateExecutionStatus();
            updatePreemption(this);
            //engine always resumes with no args
            //and since this is the top level the resume is not from any other coroutine so it is nullptr
            log::debug("resuming now");
            //luaStackDump(L);
            int status = lua_resume(L, nullptr, 0, &nRets);
            //luaStackDump(L);
            bool terminated = isTerminating(executionStatus);
            if (status != LUA_YIELD || terminated) {
                ScriptResult ret;
                if (!terminated) switch (status) {
                    case LUA_ERRRUN:
                    case LUA_ERRMEM:
                    case LUA_ERRGCMM:
                    case LUA_ERRERR:
                    case LUA_ERRFILE: //no idea how it would be ERRFILE or ERRSYNTAX but whatever
                    case LUA_ERRSYNTAX: {
                        size_t len;
                        const char* str = lua_tolstring(L, -1, &len);
                        ret = {LUA_RUN_ERROR, std::string(str, len)};
                        break;
                    }
                    case LUA_OK:
                        ret = {OK};
                        break;
                    default: break;
                } else {
                    ret = {LUA_RUN_ERROR, terminationMessage(executionStatus.type)};
                }
                lua_settop(L, 0);
                executionStatus.type = STOPPED;
                executionStatus.asyncData.isAsync = false;
                updateExecutionStatus();
                fastIsExecuting = false;

                executionStatus.asyncData.postResult(ret);

                stateSetup();
            } else {
                handleYield(this, nRets);
            }
        }
    }

    unlock();
}


LuaEngine* LuaEngine::get() {
    if (!instance) {
        instance = new LuaEngine();
        instance->init();
    }
    return instance;
}
LuaEngine* LuaEngine::get(lua_State* lua) {
    return *static_cast<LuaEngine**>(lua_getextraspace(lua));
}
const ScriptExecutionStatus& LuaEngine::getStatus() {
    return executionStatus;
}
bool LuaEngine::fastGetIsExecuting() {
    return fastIsExecuting;
}

void LuaEngine::sendInputString(const std::string& input) {
    log::debug("input sent: {}", input);
    sendToConsole(input);
    {
        std::lock_guard lock(luaInBufferMutex);

        if (input.contains('\n'))
            luaInBufferNewlineWritten = true;
        if (luaInBufferPendingCharacters > input.size()) {
            luaInBufferPendingCharacters -= input.size();
        } else luaInBufferPendingCharacters = 0;
        luaInBuffer.write(input);
    }
}
std::vector<std::string>& LuaEngine::getLines() {
    return consoleLines;
}
ScriptSelectorPopup* LuaEngine::getCurrentConsole() {
    return currentConsole;
}
bool LuaEngine::isAsynchronous() const {
    return executionStatus.asyncData.isAsync;
}


void LuaEngine::registerConsole(ScriptSelectorPopup* console) {
    currentConsole = console;
}
void LuaEngine::unregisterConsole(ScriptSelectorPopup* console) {
    if (currentConsole == console)
        currentConsole = nullptr;
}
void LuaEngine::updateScriptStatusForConsole() const {
    if (currentConsole) currentConsole->updateScriptStatus(true);
}

bool LuaEngine::tryLock() {
    bool success = !lockFlag.test_and_set(std::memory_order_acquire);
    //if the flag was already set, that means some other thread holds the lock already
    //so the lock operation is a fail
    log::debug("{}", success ? "engine locked" : "engine lock failed");
    return success;
}
void LuaEngine::unlock() {
    lockFlag.clear(std::memory_order_release);
    lockFlag.notify_all();
    log::debug("engine unlocked");
}

void LuaEngine::forceReset() {
    bool locked = tryLock();
    if (!locked) log::warn("Resetting Lua state while the engine is locked");

    currentConsole = nullptr;

    if (executionStatus.type != STOPPED) {
        lua_settop(lua.lua_state(), 0);
        executionStatus.type = STOPPED;

        if (executionStatus.asyncData.isAsync && executionStatus.asyncData.postResult)
            executionStatus.asyncData.postResult({SCRIPT_CANCELED});

        updateExecutionStatus();
        executionStatus.asyncData.isAsync = false;
        fastIsExecuting = false;

        stateSetup();
    }

    if (locked) unlock();
}

sol::state& LuaEngine::state() {
    return lua;
}
ScriptResult LuaEngine::execute(EditorUI* editor, const std::string& code, const std::string& name) {
    log::debug("executing script {}", name);
    if (!tryLock())
        return ScriptResult {ENGINE_LOCKED};
    if (executionStatus.type != STOPPED) {
        unlock();
        return ScriptResult {ENGINE_IN_USE};
    }

    editorSetup(editor);
    log::debug("editor setup complete");

    ScriptResult ret;

    {
        auto compile = lua.load(code, "="s + name, sol::load_mode::text);
        if (!compile.valid()) {
            auto err = sol::stack::get<std::string>(compile.lua_state(), compile.stack_index());
            ret = ScriptResult {LUA_COMPILE_ERROR, err};
            goto unlock; //no need to reset state as it has not been used
        }
        log::debug("compiled");
        auto func = compile.get<sol::protected_function>();

        executionStatus.scriptName = name;
        executionStatus.asyncData.isAsync = false;
        executionStatus.type = RUNNING;
        fastIsExecuting = true;
        updateExecutionStatus();
        preRunSetup(this);
        updatePreemption(this);
        log::debug("executing now");
        //luaStackDump(lua.lua_state());
        auto result = func();
        //luaStackDump(lua.lua_state());
        bool terminated = isTerminating(executionStatus);
        fastIsExecuting = false;
        executionStatus.type = STOPPED;
        updateExecutionStatus();
        if (!result.valid() || terminated) {
            std::string err;
            if (!terminated) {
                err = sol::stack::get<std::string>(result.lua_state(), result.stack_index());
            } else {
                err = terminationMessage(executionStatus.type);
            }
            ret = ScriptResult {LUA_RUN_ERROR, err};
            goto cleanup;
        }
        log::debug("execution finished");
        ret = ScriptResult {OK};
        //fall through to cleanup
    }

    cleanup:
    //reset environment and clear reference to editor
    stateSetup();
    log::debug("state cleared");
    unlock:
    unlock();
    return ret;
}
ScriptTask LuaEngine::executeAsync(EditorUI* editor, const std::string& code, const std::string& name) {
    log::debug("executing script {} async", name);
    if (!tryLock())
        return ScriptTask::immediate({ENGINE_LOCKED});
    if (executionStatus.type != STOPPED) {
        unlock();
        return ScriptTask::immediate({ENGINE_IN_USE});
    }

    editorSetup(editor);
    log::debug("editor setup complete");

    ScriptTask ret;

    lua_State* L = lua.lua_state();
    {
        //auto thread = sol::thread::create(lua.lua_state());
        lua_settop(L, 0);
        auto chunkName = "="s + name;
        int loadStatus = luaL_loadbufferx(L, code.data(), code.size(), chunkName.c_str(), "t");
        if (loadStatus != LUA_OK) {
            size_t len;
            auto err = lua_tolstring(L, -1, &len);
            ret = ScriptTask::immediate({LUA_COMPILE_ERROR, std::string(err)});
            goto unlock; //no need to reset state as it has not been used
        }
        log::debug("compiled");

        executionStatus.scriptName = name;
        executionStatus.asyncData.isAsync = true;
        executionStatus.type = RUNNING;
        fastIsExecuting = true;
        updateExecutionStatus();
        preRunSetup(this);
        updatePreemption(this);
        log::debug("executing now");
        //luaStackDump(L);
        int numRets;
        int runStatus = lua_resume(L, nullptr, 0, &numRets);
        //luaStackDump(L);
        updateExecutionStatus();
        bool terminated = isTerminating(executionStatus);
        if (runStatus == LUA_YIELD && !terminated) {
            //sol::stack::push(lua.lua_state(), result);
            //for some reason it thinks it yielded every value on the stack
            //just make it 0 idk
            handleYield(this, 0);

            auto [task, postResult, postProgress, isCancelled] =
                ScriptTask::spawn(std::string("async script: " + name));
            executionStatus.asyncData.postResult = postResult;
            executionStatus.asyncData.postProgress = postProgress;
            executionStatus.asyncData.isCancelled = isCancelled;
            executionStatus.asyncData.task = task;
            ret = task;
            goto unlock; //script is still running, do not reset the state
        } else if (runStatus == LUA_OK && !terminated) {
            //script returned without yielding
            ret = ScriptTask::immediate({OK});
            goto immediateFinish;
        } else {
            std::string err;
            if (!terminated) {
                size_t len;
                auto errStr = lua_tolstring(L, -1, &len);
                err = std::string(errStr, len);
            } else {
                err = terminationMessage(executionStatus.type);
            }
            ret = ScriptTask::immediate({LUA_RUN_ERROR, err});
            goto immediateFinish;
        }
    }

    immediateFinish:
    executionStatus.type = STOPPED;
    executionStatus.asyncData.isAsync = false;
    fastIsExecuting = false;
    updateExecutionStatus();
    //reset environment and clear reference to editor
    stateSetup();
    log::debug("state cleared");

    unlock:
    unlock();
    return ret;
}
