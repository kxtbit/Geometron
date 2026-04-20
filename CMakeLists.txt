cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
# std::atomic_flag::notify_all() apparently requires macOS 11.0+
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(Geometron VERSION 0.1.0)

# Add all source files inside src (recursively)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Set up the mod binary
add_library(${PROJECT_NAME} SHARED ${SOURCES}
        src/engine/LuaEngine.cpp
        src/engine/LuaEngine.hpp
        src/engine/LuaModules.hpp
        src/engine/LuaTypes.cpp
        src/engine/LuaUtils.cpp
        src/engine/LuaUtils.hpp
        src/engine/LuaSmartArray.cpp
        src/engine/LuaBackend.hpp
        src/engine/LuaIO.cpp
        src/engine/bindings/properties.hpp
        src/engine/bindings/properties_GameObjects.cpp
        src/engine/bindings/properties_triggers.cpp
        src/utils/types/GPoint.hpp
        src/utils/types/GTransform.hpp
        src/utils/types/FixedString.hpp
        src/utils/ScriptLoader.cpp
        src/utils/ScriptLoader.hpp
        src/utils/Settings.cpp
        src/utils/Settings.hpp
        src/utils/ResizingCircularBuffer.cpp
        src/utils/ResizingCircularBuffer.hpp
        src/utils/FontUtils.cpp
        src/utils/FontUtils.hpp
        src/ui/ScriptSelectorPopup.cpp
        src/ui/ScriptSelectorPopup.hpp
        src/ui/ScrollingWindow.cpp
        src/ui/ScrollingWindow.hpp
        src/ui/EnhancedTextPanel.cpp
        src/ui/EnhancedTextPanel.hpp
        src/hooks/TrackedGameObject.cpp
        src/hooks/TrackedGameObject.hpp
        src/hooks/TrackingEditorLayer.cpp
        src/hooks/TrackingEditorLayer.hpp)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# target_compile_options(${PROJECT_NAME} PRIVATE -O0)

add_subdirectory(external/lua)
# target_compile_definitions(lua PRIVATE LUAI_ASSERT=1)
if (WIN32)
    target_compile_definitions(lua PRIVATE LUA_USE_ASM_LONGJMP=1)
endif()
if (IOS)
    # this causes Lua to stub out the call to system() in os.execute, which is necessary because iOS doesn't support it
    target_compile_definitions(lua PRIVATE LUA_USE_IOS=1)
endif()
# target_include_directories(lua PRIVATE include)
target_link_libraries(${PROJECT_NAME} lua::lib)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/sol/sol.hpp)

#if (WIN32)
#    target_compile_definitions(${PROJECT_NAME} PRIVATE SOL_ALL_SAFETIES_ON=1)
#else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE SOL_ALL_SAFETIES_ON=1 SOL_NO_EXCEPTIONS=1)
#endif()

# Set up dependencies, resources, and link Geode.
setup_geode_mod(${PROJECT_NAME})
