// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
#include <Geode/Geode.hpp>

#include "LuaEngine.hpp"

using namespace geode::prelude;

using namespace std::string_literals;

sol::variadic_results ConsoleInFile::seek0(sol::this_state lua) {
    return illegalSeek(lua);
}
sol::variadic_results ConsoleInFile::seek1(std::string whence, sol::this_state lua) {
    return illegalSeek(lua);
}
sol::variadic_results ConsoleInFile::seek2(std::string whence, long long offset, sol::this_state lua) {
    return illegalSeek(lua);
}
YieldingFunctionCoroutine ConsoleInFile::read(sol::variadic_args args, lua_State* L) {
    const char* errorMessage = nullptr;
    {
        auto engine = LuaEngine::get(L);
        if (!engine->isAsynchronous()) {
            errorMessage = "script is not running in asynchronous mode, will not block main thread";
            goto throwError;
        }

        if (args.size() == 0) {
            sol::stack::push(L, "l");
            args = sol::variadic_args(L, lua_absindex(L, -1));
        }

        sol::variadic_results ret;
        ret.reserve(args.size());

        for (auto arg : args) {
            if (sol::optional<long long> intArg = arg; intArg.has_value()) {
                auto len = intArg.value();
                if (len == 0) {
                    ret.push_back(sol::make_object(L, ""));
                    continue;
                } else if (len < 0) {
                    errorMessage = "cannot read negative bytes";
                    goto throwError;
                }
                while (true) {
                    std::lock_guard lock(engine->luaInBufferMutex);

                    auto& inBuffer = engine->luaInBuffer;
                    size_t readable = inBuffer.readable();
                    if (readable >= intArg) {
                        std::string buf = inBuffer.read(len);
                        ret.push_back(sol::make_object(L, buf));
                        goto continueOuter;
                    }

                    engine->executionStatus.type = YIELDING;
                    engine->executionStatus.yieldType = WAITING_FOR_CONSOLE_CHARACTERS;
                    engine->luaInBufferPendingCharacters = len - readable;
                    co_yield {L, {}};
                }
            } else if (sol::optional<std::string> strArg = arg; strArg.has_value()) {
                auto str = strArg.value();
                if (str == "l" || str == "L") {
                    int i = 0;
                    std::string line;
                    while (true) {
                        std::lock_guard lock(engine->luaInBufferMutex);

                        auto& inBuffer = engine->luaInBuffer;
                        for (; i < inBuffer.readable(); i++) {
                            if (inBuffer.peek(i) == '\n') goto doRead;
                        }

                        engine->executionStatus.type = YIELDING;
                        engine->executionStatus.yieldType = WAITING_FOR_CONSOLE_NEWLINE;
                        engine->luaInBufferNewlineWritten = false;
                        co_yield {L, {}};
                        continue;

                        doRead: { //using a goto to keep the lock on the mutex
                            inBuffer.read(line, i + 1);
                            break;
                        }
                    }
                    if (str == "l") line.erase(line.end() - 1);
                    //log::info("pushing string size {}: \"{}\"", line.size(), escapeString(line));
                    ret.push_back(sol::make_object(L, line));
                    continue;
                } else if (str == "a") {
                    errorMessage = "reading all data from the console input is not allowed as it would never finish";
                    goto throwError;
                } else if (str == "n") {
                    errorMessage = "reading numerals from console input is not implemented yet :(";
                    goto throwError;
                }
            }

            errorMessage = "invalid read format";
            goto throwError;

            continueOuter: continue;
        }

        co_return {L, ret};
    }

    //Lua uses longjmp to throw an error, not C++ exceptions
    //to avoid leaking local variables, errors are thrown down here
    //so that the other variables will go out of scope and be destructed
    throwError:
    args.~variadic_args(); //manually call destructor as luaL_error will break the automatic memory management

    luaL_error(L, errorMessage);
    co_return {L, sol::variadic_results()}; //this will never happen but whatever
}
sol::variadic_results ConsoleInFile::write(sol::variadic_args args, sol::this_state lua) {
    return badFileDescriptor(lua);
}
sol::variadic_results ConsoleInFile::close(sol::this_state lua) {
    sol::variadic_results ret;
    ret.reserve(2);
    ret.push_back(sol::nil);
    ret.push_back(sol::make_object(lua, "cannot close standard file"));
    return ret;
}
sol::variadic_results ConsoleInFile::flush(sol::this_state lua) {
    return sol::variadic_results();
}

sol::variadic_results ConsoleOutFile::seek0(sol::this_state lua) {
    return illegalSeek(lua);
}
sol::variadic_results ConsoleOutFile::seek1(std::string whence, sol::this_state lua) {
    return illegalSeek(lua);
}
sol::variadic_results ConsoleOutFile::seek2(std::string whence, long long offset, sol::this_state lua) {
    return illegalSeek(lua);
}
YieldingFunctionCoroutine ConsoleOutFile::read(sol::variadic_args args, lua_State* L) {
    co_return {L, badFileDescriptor(L)};
}
sol::variadic_results ConsoleOutFile::write(sol::variadic_args args, sol::this_state lua) {
    auto engine = LuaEngine::get(lua);
    const char* badType = nullptr;
    for (auto arg : args) {
        if (sol::optional<std::string> strArg = arg; strArg.has_value()) {
            engine->onLuaWrite(strArg.value());
        } else {
            badType = lua_typename(lua, static_cast<int>(arg.get_type()));
            goto typeError;
        }
    }
    {
        sol::variadic_results ret;
        ret.reserve(1); //does reserving 1 even do anything? idk
        ret.push_back(getThisUserdata(args));
        return ret;
    }

    //jump out of scope of the rest of the function to call destructors on vars because longjmp n stuff
    typeError:
    args.~variadic_args();

    luaL_error(lua, "cannot write value of type %s", badType);
    return sol::variadic_results();
}
sol::variadic_results ConsoleOutFile::close(sol::this_state lua) {
    sol::variadic_results ret;
    ret.reserve(2);
    ret.push_back(sol::nil);
    ret.push_back(sol::make_object(lua, "cannot close standard file"));
    return ret;
}
sol::variadic_results ConsoleOutFile::flush(sol::this_state lua) {
    return sol::variadic_results();
}

template<FixedString Name>
int notImplementedFunction(lua_State* L) {
    constexpr FixedString suffix = " not implemented yet";
    constexpr FixedString str(Name, suffix);
    return luaL_error(L, str.v);
}

static YieldingFunctionCoroutine readWrapper(sol::variadic_args args, lua_State* L) {
    auto file = args[0];
    if (!file.is<CustomLuaFile*>() && !file.is<ConsoleInFile*>() && !file.is<ConsoleOutFile>()) {
        args.~variadic_args();
        file.~stack_proxy();
        luaL_error(L, "not a valid file");
    }
    file.push(L);
    //sol y u no do typ hirarcy
    auto realFile = *static_cast<CustomLuaFile**>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return realFile->read(
        sol::variadic_args(args.lua_state(), args.stack_index() + 1), L);
}

void luaAddIO(sol::state_view& lua) {
    //auto& lua = engine.lua;

    auto readFunction = yieldingFunction(lua, &readWrapper);

    auto customFileType = lua.new_usertype<CustomLuaFile>(sol::no_constructor,
        "seek", sol::overload(&CustomLuaFile::seek0, &CustomLuaFile::seek1, &CustomLuaFile::seek2),
        "read", readFunction,
        "write", &CustomLuaFile::write,
        "close", &CustomLuaFile::close,
        "flush", &CustomLuaFile::flush);
    setupImmutableMT(lua, nullptr, customFileType);
    auto consoleInFileType = lua.new_usertype<ConsoleInFile>(sol::no_constructor,
        "seek", sol::overload(&ConsoleInFile::seek0, &ConsoleInFile::seek1, &ConsoleInFile::seek2),
        "read", readFunction,
        "write", &ConsoleInFile::write,
        "close", &ConsoleInFile::close,
        "flush", &ConsoleInFile::flush);
    setupImmutableMT(lua, nullptr, consoleInFileType);
    auto consoleOutFileType = lua.new_usertype<ConsoleOutFile>(sol::no_constructor,
        "seek", sol::overload(&ConsoleOutFile::seek0, &ConsoleOutFile::seek1, &ConsoleOutFile::seek2),
        "read", readFunction,
        "write", &ConsoleOutFile::write,
        "close", &ConsoleOutFile::close,
        "flush", &ConsoleOutFile::flush);
    setupImmutableMT(lua, nullptr, consoleOutFileType);

    auto io = lua.create_table(0, 2);
    io["stdin"] = ConsoleInFile {};
    io["stdout"] = ConsoleOutFile {};
    sol::userdata luaIn = io["stdin"];
    sol::userdata luaOut = io["stdout"];
    io.set_function("close", [](lua_State* L) -> int {
        if (lua_gettop(L) < 1) {
            lua_pushnil(L);
            lua_pushstring(L, "cannot close standard file");
            return 2;
        }
        lua_settop(L, 1);
        lua_getfield(L, -1, "close");
        lua_insert(L, -2);
        lua_call(L, 1, LUA_MULTRET);
        return lua_gettop(L);
    });
    io.set_function("flush", [] {return true;});
    io.set_function("input", [=] {return luaIn;});
    io.set_function("output", [=] {return luaOut;});
    constexpr lua_KFunction readWriteCont = [](lua_State* L, int, lua_KContext) -> int {
        return lua_gettop(L);
    };
    io.set("read", sol::make_closure([](lua_State* L) -> int {
        lua_getfield(L, lua_upvalueindex(1), "read");
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_rotate(L, 1, 2);
        lua_callk(L, lua_gettop(L) - 1, LUA_MULTRET, 0, readWriteCont);
        return readWriteCont(L, LUA_OK, 0);
    }, luaIn));
    io.set("write", sol::make_closure([](lua_State* L) -> int {
        lua_getfield(L, lua_upvalueindex(1), "write");
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_rotate(L, 1, 2);
        lua_callk(L, lua_gettop(L) - 1, LUA_MULTRET, 0, readWriteCont);
        return readWriteCont(L, LUA_OK, 0);
    }, luaOut));
    io.set_function("lines", &notImplementedFunction<"io.lines">);
    io.set_function("open", &notImplementedFunction<"io.open">);
    io.set_function("popen", &notImplementedFunction<"io.popen">);
    io.set_function("tmpfile", &notImplementedFunction<"io.tmpfile">);
    io.set_function("type", &notImplementedFunction<"io.type">);

    lua["io"] = io;

    lua.set_function("print", [](lua_State* L) -> int {
        //based on luaB_print from lbaselib.c
        std::stringstream out;
        int nArgs = lua_gettop(L);
        for (int i = 1; i <= nArgs; i++) {
            size_t len;
            const char *s = luaL_tolstring(L, i, &len);
            if (i > 1) {
                //GD labels can't render tabs it seems
                //just do something that at least gets the point across
                out << "    ";
                //log::info("add tab");
            }
            auto str = std::string(s, len);
            out << str;
            //log::info("add \"{}\"", escapeString(str));
            lua_pop(L, 1);
        }
        out << '\n';
        //log::info("add newline");
        auto finalStr = out.str();
        //log::info("printing \"{}\", stringstream len {}", escapeString(finalStr), out.view().size());
        LuaEngine::get(L)->onLuaWrite(out.str());
        return 0;
    });
}