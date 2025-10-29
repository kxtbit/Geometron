// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
#include <Geode/Geode.hpp>

#include "ScriptLoader.hpp"

using namespace geode::prelude;

ScriptLoader* ScriptLoader::instance;
static void ScriptLoader_init() {
    auto saveDir = Mod::get()->getSaveDir();
    ScriptLoader::instance = new ScriptLoader(saveDir / "scripts", saveDir / "workspace");
}

$execute {
    ScriptLoader_init();
}

ScriptLoader::ScriptLoader(const std::filesystem::path& scriptDir, const std::filesystem::path& workDir) {
    this->scriptDir = scriptDir;
    this->workDir = workDir;
}
ScriptLoader* ScriptLoader::get() {
    return instance;
}

bool ScriptLoader::isLoaded() const {
    return loaded;
}


std::filesystem::path ScriptLoader::getScriptDir() {
    return scriptDir;
}
std::filesystem::path ScriptLoader::getWorkDir() {
    return workDir;
}

std::vector<ScriptEntry> ScriptLoader::getScripts() {
    //if (loaded) return scripts;

    //log::info("Loading scripts on first initialization...");

    namespace fs = std::filesystem;
    if (!fs::exists(scriptDir) && !fs::create_directory(scriptDir)) {
        log::error("Failed to create script directory, no scripts will be loaded");
        return scripts;
    }
    if (!fs::exists(workDir) && !fs::create_directory(workDir)) {
        log::error("Failed to create work directory, script filesystem will not function");
    }

    scripts.clear();
    for (auto entry : fs::directory_iterator(scriptDir)) {
        fs::path path = entry.path();
        log::debug("found file {}", path.filename().string());
        if (entry.is_directory() || !path.filename().string().ends_with(".lua"))
            continue;
        ScriptEntry script = {
            .name = path.filename().string(),
            .path = path,
        };
        log::debug("adding script {}", script.name);
        scripts.push_back(script);
    }
    loaded = true;

    return scripts;
}

std::optional<std::string> ScriptLoader::loadScript(const std::string& name) const {
    namespace fs = std::filesystem;

    auto path = scriptDir / name;
    if (!fs::exists(path) || fs::is_directory(path)) {
        log::error("The script file at {} does not exist or is a directory, cannot load", path);
        return std::nullopt;
    }

    std::ifstream file(path, std::ios::binary);
    if (file.fail() || !file.is_open()) {
        log::error("Failed to open script file {}: {}", name, strerror(errno));
        return std::nullopt;
    }
    std::string data;
    file.seekg(0, std::ios::end);
    data.resize(file.tellg());
    file.seekg(0, std::ios::beg);

    if (!file.read(data.data(), data.size())) {
        log::error("Failed to read script file {}: {}", name, strerror(errno));
        return std::nullopt;
    }
    file.close();

    return data;
}

