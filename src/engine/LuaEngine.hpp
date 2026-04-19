// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppParameterMayBeConst
#pragma once
// ReSharper disable CppMemberFunctionMayBeStatic

#ifndef GEOMETRON_LUAENGINE_HPP
#define GEOMETRON_LUAENGINE_HPP

#include <numbers>

#include <Geode/Geode.hpp>

#include "LuaBackend.hpp"
#include "LuaEngine.hpp"

#include "../ui/ScriptSelectorPopup.hpp"
#include "../utils/ResizingCircularBuffer.hpp"

using namespace geode::prelude;

enum ScriptResultType : int {
    OK,
    LUA_RUN_ERROR,
    LUA_COMPILE_ERROR,
    SCRIPT_CANCELED,
    ENGINE_LOCKED,
    ENGINE_IN_USE,
};
struct ScriptResult {
    ScriptResultType result;
    std::string errorMessage;
};
enum ScriptExecutionType : int {
    RUNNING,
    STOPPED,
    YIELDING,
    TERMINATING_TIMEOUT,
    TERMINATING_CANCELED,
};
enum ScriptYieldType : int {
    SLEEPING,
    WAITING_FOR_CONSOLE_NEWLINE,
    WAITING_FOR_CONSOLE_CHARACTERS,
    PREEMPTED,
    MANUAL,
};
struct ScriptExecutionStatus;
struct ScriptTask {
    bool hasResult = false;
    ScriptResult result;
    bool isCancelled;

    geode::CopyableFunction<void(ScriptResult)> onResult;
    geode::CopyableFunction<void(const ScriptExecutionStatus*)> onProgress;

    void cancel() {
        isCancelled = true;
    }

    void setOnResult(const geode::CopyableFunction<void(ScriptResult)>& callback) {
        onResult = callback;
        if (hasResult) {
            callback(result);
        }
    }

    void postResult(const ScriptResult& newResult) {
        hasResult = true;
        result = newResult;
        if (onResult) {
            onResult(newResult);
        }
    }
    void postProgress(const ScriptExecutionStatus* progress) const {
        if (onProgress) {
            onProgress(progress);
        }
    }

    static std::shared_ptr<ScriptTask> create(const ScriptTask& value) {
        auto ptr = std::make_shared<ScriptTask>();
        *ptr = value;
        return std::move(ptr);
    }
    static ScriptTask immediate(const ScriptResult& result) {
        return ScriptTask {true, result, false};
    }
};
struct ScriptExecutionStatus {
    ScriptExecutionType type;
    ScriptYieldType yieldType;
    std::string scriptName;
    union {
        float remainingSleepDuration;
    } yieldData;
    struct {
        bool isAsync;
        std::shared_ptr<ScriptTask> task;

        friend class LuaEngine;
    } asyncData;
    std::chrono::time_point<std::chrono::steady_clock> resumeTime;
};

class LuaEngine {
    static LuaEngine* instance;

    std::atomic_flag lockFlag;

    std::atomic_bool fastIsExecuting = false;
    ScriptExecutionStatus executionStatus = {STOPPED};

    bool tryLock();
    void unlock();

    void init();
    void stateSetup();
    void stateDestroy();
    void editorSetup(EditorUI* editor);

    Ref<ScriptSelectorPopup> currentConsole;
    std::vector<std::string> consoleLines;
    void sendToConsole(const std::string& text);

    std::recursive_mutex luaInBufferMutex; //recursive_mutex probably not necessary here but i don't want to deadlock anything
    bool luaInBufferNewlineWritten = false;
    unsigned long long luaInBufferPendingCharacters = 0;
    ResizingCircularBuffer<char> luaInBuffer;
    void onLuaWrite(const std::string& data);

    void updateExecutionStatus() const;

    Ref<CCObject> updater;
    void update(float dt);

    void tryResume();

    friend class ConsoleInFile;
    friend class ConsoleOutFile;
    template<int (*ReturnHandler)(lua_State*, int)>
    friend int doPostResume(lua_State*, lua_State*, int, int);
    friend void handleYield(LuaEngine*, int);
    friend void luaPreemptionHook(lua_State*, lua_Debug*);
    friend void updatePreemption(LuaEngine*);
    friend void preRunSetup(LuaEngine*);

    friend void luaAddIO(sol::state_view&);
public:
    sol::state lua;
    Ref<EditorUI> editor;

    static LuaEngine* get();
    static LuaEngine* get(lua_State* lua);

    static bool isInitialized();

    const ScriptExecutionStatus& getStatus();
    bool fastGetIsExecuting();

    void sendInputString(const std::string& input);
    std::vector<std::string>& getLines();
    ScriptSelectorPopup* getCurrentConsole() const;
    bool isAsynchronous() const;

    void registerConsole(ScriptSelectorPopup* console);
    void unregisterConsole(ScriptSelectorPopup* console);
    void updateScriptStatusForConsole() const;

    void forceReset();

    sol::state& state();
    ScriptResult execute(EditorUI* editor, const std::string& code, const std::string& name = "<unnamed script>");
    std::shared_ptr<ScriptTask> executeAsync(EditorUI* editor, const std::string& code, const std::string& name = "<unnamed script>");
};

using curengine = LuaEngine*;

struct YieldingFunctionResults {
    lua_State* state;
    sol::variadic_results results;
};

struct YieldingFunctionPromise;
struct YieldingFunctionCoroutine : std::coroutine_handle<YieldingFunctionPromise> {
    using promise_type = YieldingFunctionPromise;
};
struct YieldingFunctionPromise {
    int stackStart;
    int stackEnd;
    std::exception_ptr exception_;
    YieldingFunctionCoroutine get_return_object() {
        return {YieldingFunctionCoroutine::from_promise(*this)};
    }
    std::suspend_always initial_suspend() {
        return {};
    }
    std::suspend_always final_suspend() noexcept {
        return {};
    }
    void unhandled_exception() {
        exception_ = std::current_exception();
    }

    std::suspend_always yield_value(YieldingFunctionResults value) {
        stackStart = lua_gettop(value.state);
        for (auto result : value.results) {
            if (!lua_checkstack(value.state, sol::lua_size_v<decltype(result)>)) break;
            sol::stack::push(value.state, result);
        }
        stackEnd = lua_gettop(value.state);
        return {};
    }
    void return_value(YieldingFunctionResults value) {
        yield_value(value);
    }
};
template<class F>
sol::function yieldingFunction(sol::state_view& lua, F func);

class CustomLuaFile {
protected:
    static sol::object getThisUserdata(const sol::variadic_args& args) {
        auto newArgs = sol::variadic_args(args.lua_state(), 1);
        return newArgs.get<sol::object>(0);
    }
    static sol::variadic_results badFileDescriptor(lua_State* L) {
        sol::variadic_results ret;
        ret.reserve(3);
        ret.push_back(sol::nil);
        ret.push_back(sol::make_object(L, "Bad file descriptor"));
        ret.push_back(sol::make_object(L, 9));
        return ret;
    }
    static sol::variadic_results illegalSeek(lua_State* L) {
        sol::variadic_results ret;
        ret.reserve(3);
        ret.push_back(sol::nil);
        ret.push_back(sol::make_object(L, "Illegal seek"));
        ret.push_back(sol::make_object(L, 29));
        return ret;
    }
public:
    virtual sol::variadic_results seek0(sol::this_state lua) = 0;
    virtual sol::variadic_results seek1(std::string whence, sol::this_state lua) = 0;
    virtual sol::variadic_results seek2(std::string whence, long long offset, sol::this_state lua) = 0;
    virtual YieldingFunctionCoroutine read(sol::variadic_args args, lua_State* L) = 0;
    virtual sol::variadic_results write(sol::variadic_args args, sol::this_state lua) = 0;

    virtual sol::variadic_results close(sol::this_state lua) = 0;
    virtual sol::variadic_results flush(sol::this_state lua) = 0;

    virtual ~CustomLuaFile() {}
};

class ConsoleInFile : public CustomLuaFile {
public:
    sol::variadic_results seek0(sol::this_state lua) override;
    sol::variadic_results seek1(std::string whence, sol::this_state lua) override;
    sol::variadic_results seek2(std::string whence, long long offset, sol::this_state lua) override;
    YieldingFunctionCoroutine read(sol::variadic_args args, lua_State* L) override;
    sol::variadic_results write(sol::variadic_args args, sol::this_state lua) override;

    sol::variadic_results close(sol::this_state lua) override;
    sol::variadic_results flush(sol::this_state lua) override;
};
class ConsoleOutFile : public CustomLuaFile {
public:
    sol::variadic_results seek0(sol::this_state lua) override;
    sol::variadic_results seek1(std::string whence, sol::this_state lua) override;
    sol::variadic_results seek2(std::string whence, long long offset, sol::this_state lua) override;
    YieldingFunctionCoroutine read(sol::variadic_args args, lua_State* L) override;
    sol::variadic_results write(sol::variadic_args args, sol::this_state lua) override;

    sol::variadic_results close(sol::this_state lua) override;
    sol::variadic_results flush(sol::this_state lua) override;
};

namespace sol {
    template<>
    struct is_automagical<GameObject> : std::false_type {};
    template<>
    struct is_transparent_argument<curengine> : std::true_type {};
    namespace stack {
        template<>
        struct unqualified_getter<curengine> {
            static curengine get(lua_State* L, int, record& tracking) {
                tracking.use(0);
                return curengine(LuaEngine::get(L));
            }
        };
        template <>
        struct unqualified_pusher<curengine> {
            static int push(lua_State*, const curengine&) noexcept {
                return 0;
            }
        };
    }
    namespace detail {
        template <>
        struct lua_type_of<curengine> : std::integral_constant<type, type::poly> {};
    }
    template<typename T>
    struct unique_usertype_traits<geode::Ref<T>> {
        typedef T type;
        typedef geode::Ref<T> actual_type;
        static const bool value = true;

        static bool is_null(const actual_type& ptr) {
            return ptr == nullptr;
        }

        static type* get(const actual_type& ptr) {
            return ptr.data();
        }
    };
}
template <typename Handler>
bool sol_lua_check(sol::types<GameObject*>, lua_State* L, int index, Handler&& handler, sol::stack::record& tracking) {
    if (lua_isnoneornil(L, index)) {
        handler(L, index, sol::type_of(L, index), sol::type::userdata, "expected non-nil object");
        return false;
    } else if (!sol::stack::check_usertype<GameObject>(L, index)) {
        handler(L, index, sol::type_of(L, index), sol::type::userdata, "expected GameObject");
        return false;
    }
    tracking.use(1);
    return true;
}

#endif //GEOMETRON_LUAENGINE_HPP