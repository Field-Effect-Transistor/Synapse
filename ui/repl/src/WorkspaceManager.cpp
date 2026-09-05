//  /ui/repl/src/WorkspaceManager.cpp
#include "WorkspaceManager.hpp"

#include <iostream>
#include <filesystem>
#include <stdlib.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>

#ifndef _WIN32
    //  posix
    #include <unistd.h>
    #include <sys/wait.h>
#endif

//  synapse
#include <synapse/DynamicLibrary.hpp>

using json      = nlohmann::json;
namespace fs    = std::filesystem;

#ifdef _WIN32
    constexpr const char* PLUGIN_EXT = ".dll";
#else
    constexpr const char* PLUGIN_EXT = ".so";
#endif

namespace {
    

    bool erase(std::vector<std::string>& vector, const std::string& str) {
        if(const auto& it = std::find(vector.begin(), vector.end(), str); it != vector.end()) {
            vector.erase(it);
            return true;
        }
        return false;
    }

};

namespace Synapse {

    void to_json(json& j, const Calculator::Recipe& r) {
        j = json {
            {"lexer",       r.lexer},
            {"parser",      r.parser},
            {"producer",    r.producer},
            {"preprocessors",   r.preprocessors}
        };
    }

    void from_json(const json& j, Calculator::Recipe& r) {
        r.lexer     = j.value("lexer",      "");
        r.parser    = j.value("parser",     "");
        r.producer  = j.value("producer",   "");

        r.preprocessors.clear();
        if (j.contains("preprocessors") && j["preprocessors"].is_array()) {
            for (const auto& it : j["preprocessors"]) {
                r.preprocessors.push_back(it.get<std::string>());
            }
        }
    }

}   //  namespace   Synapse

namespace Synapse::Repl {

    struct WorkspaceManager::AppConfig {
        std::vector<std::string>    autoload_plugins;
        std::vector<std::string>    quarantined_plugins;
        std::string                 quarantine_pending;
    };  //  WorkspaceManager::AppConfig

    void to_json(json& j, const WorkspaceManager::AppConfig& a) {
        j = json {
            {"autoloaded_plugins",  a.autoload_plugins},
            {"quarantined_plugins", a.quarantined_plugins},
            {"quarantine_pending",  a.quarantine_pending}
        };
    }

    void from_json(const json& j, WorkspaceManager::AppConfig& a) {
        a.autoload_plugins      = j.value("autoloaded_plugins",     std::vector<std::string>{});
        a.quarantined_plugins   = j.value("quarantined_plugins",    std::vector<std::string>{});
        a.quarantine_pending    = j.value("quarantine_pending",     "");
    }


    WorkspaceManager::WorkspaceManager() {
        const char* home = std::getenv("HOME");
        if (!home) {
            home = ".";
        }

        fs::path root = fs::path(home) / ".synapse";
        
        _root_dir       = root.string();
        _plugins_dir    = (root / "plugins").string();
        _recipes_dir    = (root / "recipes").string();
        _config_file    = (root / "config.json").string();
        
        _initDirectories();
    }


    //  ------------------------
    //      PRIVATE METHODS
    //  ------------------------

    void WorkspaceManager::_initDirectories() const {
        try {
            fs::create_directories(_plugins_dir);
            fs::create_directories(_recipes_dir);

            if (!fs::exists(_config_file)) {
                _writeConfig({});
            }

        } catch (const fs::filesystem_error& e) {
            throw WorkspaceError("Failed to init workspace: " + std::string(e.what()));
        }
    }

    WorkspaceManager::PluginCheckResult WorkspaceManager::checkPlugin(const std::string& path) const {
        if (!fs::exists(path)) {
            throw WorkspaceError("Cannot check plugin: File does not exist at '" + path + "'");
        }

#ifndef _WIN32
        pid_t process_id = fork();
        switch (process_id) {
            case 0: {   //  child
                try {
                    {
                        alarm(2);
                        DynamicLibrary lib(path.c_str());
                        IPlugin::Ptr plugin = lib.getSymbol();
                    }
                    _exit(0);
                } catch (const std::exception& e) {
                    std::cerr << "\n[Sandbox Debug] Load error: " << e.what() << "\n";
                    _exit(1);
                } catch (...) {
                    _exit(1);
                }
            }

            case -1: {  //  error
                return PluginCheckResult::SandboxError;
            }

            default: {  //  parent
                int status;
                waitpid(process_id, &status, 0);
                
                if (WIFEXITED(status)) {
                    if (WEXITSTATUS(status) == 0) {
                        return PluginCheckResult::Passed;
                    } else {
                        return PluginCheckResult::LoadError;
                    }
                } else if (WIFSIGNALED(status)) {
                    return PluginCheckResult::Crashed;
                }
            }

            return PluginCheckResult::SandboxError;
        }
#endif
        return PluginCheckResult::SandboxError;
    }

    WorkspaceManager::AppConfig WorkspaceManager::_readConfig() const {
        std::ifstream config_file(_config_file);
        
        if (!config_file.is_open()) {
            std::cerr << "Warning: Config file " << _config_file << " does not exist. Initializing workspace.\n";
            _initDirectories();
            return AppConfig{};
        }

        AppConfig config_struct;
        try {
            json config_json;
            config_file >> config_json;
            config_struct = config_json.get<AppConfig>();
        } 
        catch (const json::exception& e) {
            std::cerr << "Warning: Config file " << _config_file << " is invalid or broken: " << e.what() << "\n";
            std::cerr << "Proceeding with an empty configuration.\n";
            return AppConfig{}; 
        }

        return config_struct;
    }

    void WorkspaceManager::_writeConfig(const WorkspaceManager::AppConfig& config) const {
        std::ofstream config_file(_config_file);
        
        if (!config_file.is_open()) {
            throw WorkspaceError("Failed to write to config file: " + _config_file);
        }

        try {
            json config_json = config;
            config_file << config_json.dump(4) << std::endl;
            
        } catch (const std::exception& e) {
            throw WorkspaceError("Failed to serialize config: " + std::string(e.what()));
        }
    }


    //  ------------------------
    //      Plugin Control      
    //  ------------------------

    std::vector<std::string> WorkspaceManager::getAutoloadedPlugins() const {
        AppConfig config_struct = _readConfig();
        std::vector<std::string> res;
        res.reserve(config_struct.autoload_plugins.size());

        for (const auto& plugin_stem : config_struct.autoload_plugins) {
            res.push_back((fs::path(_plugins_dir) / (plugin_stem + PLUGIN_EXT)).string());
        }
        return res;
    }

    std::vector<std::string> WorkspaceManager::getInstalledPlugins() const {
        std::vector<std::string> res;
        for(const auto& entry : fs::directory_iterator(_plugins_dir)) {
            if (entry.is_regular_file() && entry.path().extension().string() == PLUGIN_EXT) {
                res.push_back(entry.path().stem().string());
            }
        }
        return res;
    }

    std::vector<std::string> WorkspaceManager::getQuarantinedPlugins() const {
        AppConfig config_struct = _readConfig();
        return config_struct.quarantined_plugins;
    }

    bool WorkspaceManager::enableAutoload(const std::string& plugin_name) const {
        AppConfig config_struct = _readConfig();
        auto& plugins = config_struct.autoload_plugins;
        if(std::find(plugins.begin(), plugins.end(), plugin_name) == plugins.end()) {
            plugins.push_back(plugin_name);
            _writeConfig(config_struct);
            return true;
        }
        return false;
    }

    bool WorkspaceManager::disableAutoload(const std::string& plugin_name) const {
        AppConfig config_struct = _readConfig();
        
        if (erase(config_struct.autoload_plugins, plugin_name)) {
            _writeConfig(config_struct);
            return true;
        }
        return false;
    }

    void WorkspaceManager::installPlugin(const std::string& source_path, bool auto_load) const {
        fs::path src = fs::path(source_path);
        if(!fs::exists(src)) throw WorkspaceError("Plugin " + src.string() + " doesn`t exist");
        
        std::string plugin_stem = src.stem().string(); 
        
        fs::path target_path = fs::path(_plugins_dir) / (plugin_stem + PLUGIN_EXT);

        if (fs::exists(target_path)) throw WorkspaceError("Plugin " + plugin_stem + " already exists");

        std::error_code ec;
        fs::copy_file(src, target_path, ec);

        if (ec) throw WorkspaceError("Failed to install plugin: " + ec.message());

        if (auto_load) enableAutoload(plugin_stem); 
    }

    void WorkspaceManager::uninstallPlugin(const std::string& plugin_filename) const {
        AppConfig config_struct = _readConfig();
        bool config_changed = false;
        
        config_changed |= erase(config_struct.autoload_plugins, plugin_filename);
        config_changed |= erase(config_struct.quarantined_plugins, plugin_filename);
        if (config_struct.quarantine_pending == plugin_filename) {
            config_struct.quarantine_pending.erase();
            config_changed = true;
        }

        if (config_changed) {
            _writeConfig(config_struct);
        }

        if (fs::path plugin_path = fs::path(_plugins_dir) / (plugin_filename + PLUGIN_EXT); fs::exists(plugin_path)) {
            std::error_code ec;
            fs::remove(plugin_path, ec);

            if (ec) {
                throw WorkspaceError("Failed to uninstall plugin " + plugin_path.string() + " via " + ec.message());
            }
        }
    }

    std::optional<std::string> WorkspaceManager::checkAndResolveQuarantine() const {
        AppConfig config_struct = _readConfig();

        std::string plugin;
        if (config_struct.quarantine_pending.empty()) {
            return std::nullopt;
        } else {
            plugin = std::move(config_struct.quarantine_pending);
            config_struct.quarantine_pending.clear();   //  if called copy= instead move=
        }

        auto& q = config_struct.quarantined_plugins;
        if (std::find(q.begin(), q.end(), plugin) == q.end()) {
            q.push_back(plugin);
        }
        erase(config_struct.autoload_plugins, plugin);

        _writeConfig(config_struct);
        return plugin;
    }

    void WorkspaceManager::markForTesting(const std::string& plugin_name) const {
        AppConfig config_struct = _readConfig();
        config_struct.quarantine_pending = plugin_name;
        _writeConfig(config_struct);
    }

    void WorkspaceManager::clearTestingMark() const {
        AppConfig config_struct = _readConfig();
        if (!config_struct.quarantine_pending.empty()) {
            config_struct.quarantine_pending.clear();
            _writeConfig(config_struct);
        }
    }


    //  ------------------------
    //      Recipe Control      
    //  ------------------------

    std::vector<std::string> WorkspaceManager::getAvailableRecipes() const {
        std::vector<std::string> res;
        for(const auto& entry : fs::directory_iterator(_recipes_dir)) {
            if (
                entry.is_regular_file() &&
                entry.path().extension().string() == ".json"
            ) {
                res.push_back(entry.path().stem().string());
            }
        }
        return res;
    }

    Calculator::Recipe WorkspaceManager::loadRecipe(const std::string& name) const {
        fs::path recipe_path = fs::path(_recipes_dir) / name += ".json";
        
        if (!fs::exists(recipe_path)) {
            throw WorkspaceError("Recipe load failed: recipe doesn`t exist.");
        }

        std::ifstream recipe_file(recipe_path);
        Calculator::Recipe recipe_struct;
        try {
            json recipe_json;
            recipe_file >> recipe_json;
            recipe_struct = recipe_json.get<Calculator::Recipe>();
        } catch (const std::exception& e) {
            throw WorkspaceError(std::string("Recipe load failed: ") + e.what());
        }

        return recipe_struct;
    }

    void WorkspaceManager::saveRecipe(const std::string& name, const Calculator::Recipe& recipe) const {
        fs::path recipe_path = fs::path(_recipes_dir) / name += ".json";
        std::ofstream recipe_file(recipe_path);

        try {
            json recipe_json = recipe;
            recipe_file << recipe_json.dump(4);
        } catch (const std::exception& e) {
            throw WorkspaceError(std::string("Recipe save failed: ") + e.what());
        }
    }

    void WorkspaceManager::deleteRecipe(const std::string& name) const {
        fs::path recipe_path = fs::path(_recipes_dir) / name += ".json";
        if (!fs::exists(recipe_path)) {
            throw WorkspaceError("Recipe deletion failed: recipe doesn`t exists");
        }

        std::error_code ec;
        fs::remove(recipe_path, ec);

        if (ec) {
            throw WorkspaceError("Recipe deletion failed: " + ec.message());
        }
    }
    

    //  ------------
    //      Global
    //  ------------

    void WorkspaceManager::resetWorkspace() {
        _writeConfig({});

        try {
            for (const auto& entry : fs::directory_iterator(_plugins_dir)) {
                fs::remove_all(entry.path());
            }
            for (const auto& entry : fs::directory_iterator(_recipes_dir)) {
                fs::remove_all(entry.path());
            }
        } catch (const std::exception& e) {
            throw WorkspaceError(std::string("Workspace reset failed: ") + e.what());
        }
    }

}   //  namespace   Synapse::Repl
