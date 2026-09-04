//  /ui/repl/include/WorkspaceManager.hpp
#pragma once

#include "synapse/Calculator.hpp"

#include <string>
#include <vector>
#include <optional>

namespace Synapse::Repl {

    class WorkspaceError : public std::runtime_error {
    public:
        explicit WorkspaceError(const std::string& msg) : std::runtime_error("Workspace Error: " + msg) {}
    };  //  class   WorkspaceError;


    class WorkspaceManager final {
    public:
        struct AppConfig;

        enum class PluginCheckResult : uint8_t {
            Passed,
            LoadError,
            Crashed,
            SandboxError
        };  //  enum    PluginCheckResult

    private:
        std::string _root_dir;      //  ./.synapse
        std::string _plugins_dir;   //  ./.synapse/plugins
        std::string _recipes_dir;   //  ./.synapse/recipes
        std::string _config_file;   //  ./.synapse/config.json

        void _initDirectories() const;

        AppConfig _readConfig() const;
        void _writeConfig(const AppConfig& config) const;

    public:
        WorkspaceManager();
        ~WorkspaceManager() = default;

        WorkspaceManager(const WorkspaceManager&) = delete;
        WorkspaceManager(WorkspaceManager&&) = delete;

        WorkspaceManager& operator=(const WorkspaceManager&) = delete;
        WorkspaceManager& operator=(WorkspaceManager&&) = delete;

        const std::string& getPluginsDir() const {
            return _plugins_dir;
        }

        //  ------------------------
        //      Plugin Control      
        //  ------------------------

        PluginCheckResult checkPlugin(const std::string& path) const;

        std::vector<std::string> getAutoloadedPlugins() const;
        std::vector<std::string> getQuarantinedPlugins() const;
        std::vector<std::string> getInstalledPlugins() const;
        
        bool enableAutoload(const std::string& plugin_name) const;
        bool disableAutoload(const std::string& plugin_name) const;

        void installPlugin(const std::string& source_path, bool auto_load = false) const;
        void uninstallPlugin(const std::string& plugin_filename) const;

        std::optional<std::string> checkAndResolveQuarantine() const;
        void markForTesting(const std::string& plugin_name) const;
        void clearTestingMark() const;

        //  ------------------------
        //      Recipe Control
        //  ------------------------

        std::vector<std::string> getAvailableRecipes() const;
        Calculator::Recipe loadRecipe(const std::string& name) const;
        void saveRecipe(const std::string& name, const Calculator::Recipe& recipe) const;
        void deleteRecipe(const std::string& name) const;


        //  ------------
        //      Global  
        //  ------------

        void resetWorkspace();

    };  //  class   WorkspaceManager

}   //  namespace   Synapse::Repl
