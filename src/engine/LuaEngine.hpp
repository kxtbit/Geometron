#pragma once
// ReSharper disable CppMemberFunctionMayBeStatic

#ifndef GEOMETRON_LUAENGINE_HPP
#define GEOMETRON_LUAENGINE_HPP

#include <Geode/Geode.hpp>

#include <sol/sol.hpp>

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
using ScriptTask = Task<ScriptResult, const ScriptExecutionStatus*>;
struct ScriptExecutionStatus {
    ScriptExecutionType type;
    ScriptYieldType yieldType;
    std::string scriptName;
    union {
        float remainingSleepDuration;
    } yieldData;
    struct {
        bool isAsync;
        ScriptTask task;
    protected:
        ScriptTask::PostResult postResult;
        ScriptTask::PostProgress postProgress;
        ScriptTask::HasBeenCancelled isCancelled;

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
    template<int (*Continuation)(lua_State*, int, lua_KContext)>
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

    const ScriptExecutionStatus& getStatus();
    bool fastGetIsExecuting();

    void sendInputString(const std::string& input);
    std::vector<std::string>& getLines();
    ScriptSelectorPopup* getCurrentConsole();
    bool isAsynchronous() const;

    void registerConsole(ScriptSelectorPopup* console);
    void unregisterConsole(ScriptSelectorPopup* console);
    void updateScriptStatusForConsole() const;

    void forceReset();

    sol::state& state();
    ScriptResult execute(EditorUI* editor, const std::string& code, const std::string& name = "<unnamed script>");
    ScriptTask executeAsync(EditorUI* editor, const std::string& code, const std::string& name = "<unnamed script>");
};

struct LuaPoint {
    double x;
    double y;

    double dot(LuaPoint other) const;
    LuaPoint unit() const;

    LuaPoint operator+(LuaPoint r) const;
    LuaPoint operator-(LuaPoint r) const;
    LuaPoint operator*(LuaPoint r) const;
    LuaPoint operator*(double r) const;
    LuaPoint operator/(LuaPoint r) const;
    LuaPoint operator/(double r) const;

    explicit operator std::string() const;
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
            lua_checkstack(value.state, sol::lua_size_v<decltype(result)>);
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
    struct is_automagical<LuaPoint> : std::false_type {};
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

sol::object wrapGameObject(sol::state_view& lua, GameObject* object);
sol::table wrapArrayOfGameObjects(sol::state_view& lua, cocos2d::CCArray* objects);

void updateObjectPosition(EditorUI* self, GameObject* object);

std::string escapeString(const std::string& str);

void addProtectedMT(sol::state_view& lua, std::string_view name);
inline static const std::string protectedMTMessage = "this metatable is protected";
template<class T>
void setupImmutableMT(sol::state_view& lua, auto object, sol::usertype<T> ut) {
    if constexpr (!std::is_same_v<decltype(object), std::nullptr_t>) {
        sol::optional<sol::table> omt = object[sol::metatable_key];
        auto mt = omt.has_value() ? omt.value() : lua.create_table(0, 2);
        mt["__newindex"] = sol::make_object(lua, sol::as_function([](lua_State* L) -> int {
            return luaL_error(L, "cannot change this object");
        }));
        mt["__metatable"] = protectedMTMessage;
        if (!omt.has_value()) object[sol::metatable_key] = mt;
    }

    //there is probably a better way to do this part but i am too dumb
    lua_checkstack(lua.lua_state(), 3);
    addProtectedMT(lua, sol::usertype_traits<T>::metatable());
    addProtectedMT(lua, sol::usertype_traits<T>::user_metatable());
    addProtectedMT(lua, sol::usertype_traits<const T>::metatable());
    addProtectedMT(lua, sol::usertype_traits<sol::d::u<T>>::metatable());
    addProtectedMT(lua, sol::usertype_traits<T*>::metatable());
    addProtectedMT(lua, sol::usertype_traits<T const*>::metatable());
}

void luaAddIO(sol::state_view& lua);
void luaAddEnums(sol::state_view& lua);
void luaAddUsertypes(sol::state_view& lua, EditorUI* self);

#endif //GEOMETRON_LUAENGINE_HPP