#pragma once

#ifndef GEOMETRON_SCRIPTLOADER_HPP
#define GEOMETRON_SCRIPTLOADER_HPP

struct ScriptEntry {
    std::string name;
    std::filesystem::path path;
};


class ScriptLoader {
protected:
    static ScriptLoader* instance;

    std::filesystem::path scriptDir;
    std::filesystem::path workDir;
    std::vector<ScriptEntry> scripts;
    bool loaded = false;

    explicit ScriptLoader(const std::filesystem::path& scriptDir, const std::filesystem::path& workDir);

    friend void ScriptLoader_init();
public:
    static ScriptLoader* get();

    bool isLoaded() const;

    std::filesystem::path getScriptDir();
    std::filesystem::path getWorkDir();
    std::vector<ScriptEntry> getScripts();

    std::optional<std::string> loadScript(const std::string& name) const;
};

#endif //GEOMETRON_SCRIPTLOADER_HPP