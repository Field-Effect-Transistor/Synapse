//  /ui/repl/src/ReplApp.cpp
#include "ReplApp.hpp"

#include "CommandDispatcher.hpp"
#include "WorkspaceManager.hpp"
#include "synapse/SynapseEngine.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <algorithm>

namespace Synapse::Plugin {
    extern "C" IPlugin* create_stdlib_plugin();
}

namespace Synapse::Repl {

    const char* RED     = "\033[31m";
    const char* GREEN   = "\033[32m";
    const char* YELLOW  = "\033[33m";
    const char* CYAN    = "\033[36m";
    const char* MAGENTA = "\033[35m";
    const char* RESET   = "\033[0m";

    struct ReplApp::Impl {
        CommandDispatcher           _dispatcher;
        SynapseEngine               _engine;
        WorkspaceManager            _workspace;
        std::vector<std::string>    _history;

        void _setupEngine();
        void _setupDispatcher();

        void _printHelp() const;
        void _printPlugins() const;
        void _printTools() const;
        void _printEnvironment() const;
    };

    ReplApp::ReplApp() : _impl(new Impl) {}
    ReplApp::~ReplApp() = default;

    void ReplApp::run() {
        _impl->_setupEngine();
        _impl->_setupDispatcher();

        std::cout << CYAN << "=================================================\n";
        std::cout << "  Synapse Modular Calculator [Version 1.0 MVP]   \n";
        std::cout << "  Type :help for the list of commands.           \n";
        std::cout << "=================================================\n\n" << RESET;

        std::string input;
        while (true) {
            std::string session_name = _impl->_engine.getActiveSessionName();
            if (session_name.empty()) {
                std::cout << YELLOW << "synapse:(no session)> " << RESET;
            } else {
                std::cout << GREEN << "synapse:" << session_name << "> " << RESET;
            }

            if (!std::getline(std::cin, input)) {
                std::cout << "\nExiting Synapse...\n";
                break;
            }

            size_t first_char = input.find_first_not_of(" \t");
            if (first_char == std::string::npos) continue;

            _impl->_history.push_back(input);

            try {
                if (input[first_char] == ':') {
                    std::string cmd = input.substr(first_char);
                    if (cmd == ":quit" || cmd == ":q") break;
                    
                    if (!_impl->_dispatcher.dispatch(cmd)) {
                        throw std::runtime_error("Unknown command. Type :help");
                    }
                } 
                else {
                    if (session_name.empty()) {
                        throw std::runtime_error("No active session. Use ':session new <name>'");
                    }
                    Value result = _impl->_engine.evaluate(input);
                    std::cout << "ans = " << result.to_str() << "\n";
                }
            } catch (const std::exception& e) {
                std::cerr << RED << "Error: " << e.what() << RESET << "\n";
            }
        }
    }
    
    //  --------------------
    //      IMPL METHODS    
    //  --------------------

    void ReplApp::Impl::_setupEngine() {
        auto bad_plugin = _workspace.checkAndResolveQuarantine();
        if (bad_plugin.has_value()) {
            std::cerr << RED << "\n[WARNING] Plugin '" << bad_plugin.value() 
                      << "' caused a CRASH during the last session!\n"
                      << "It has been moved to quarantine and removed from autoload.\n\n" << RESET;
        }

        _engine.loadPlugin(IPlugin::Ptr(Plugin::create_stdlib_plugin()));

        auto autoload_list = _workspace.getAutoloadedPlugins();
        for (const auto& path : autoload_list) {
            std::string filename = std::filesystem::path(path).filename().string();
            try {
                _workspace.markForTesting(filename);
                _engine.loadPluginFromFile(path.c_str());
                _workspace.clearTestingMark();
            } catch (const std::exception& e) {
                std::cerr << YELLOW << "[WARNING] Failed to autoload '" << filename << "': " << e.what() << RESET << "\n";
                _workspace.clearTestingMark();
            }
        }

        if (_engine.getAvailableSessions().empty()) {
            Calculator::Recipe default_recipe{"stdlib.Standard Lexer", "stdlib.Standard Parser", "stdlib.Math Evaluator"};
            _engine.createSession("main", default_recipe);
        }
    }

    void ReplApp::Impl::_setupDispatcher() {
        
        //  ========================
        //      SESSION CONTROL
        //  ========================
        auto cmd_session_list = [this](const auto&) {
            auto sessions = _engine.getAvailableSessions();
            if (sessions.empty()) {
                std::cout << "No sessions available.\n";
                return;
            }
            std::string active = _engine.getActiveSessionName();
            for (const auto& s : sessions) {
                std::cout << (s == active ? GREEN + std::string(" * ") : "   ") << s << RESET << "\n";
            }
        };
        _dispatcher.registerCommand(":session list", cmd_session_list);
        _dispatcher.registerCommand(":session", cmd_session_list);

        _dispatcher.registerCommand(":session new", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :session new <session_name> [recipe_name]");
            
            Calculator::Recipe recipe;
            if (args.size() >= 2) {
                recipe = _workspace.loadRecipe(args[1]);
            } else {
                recipe = {"stdlib.Standard Lexer", "stdlib.Standard Parser", "stdlib.Math Evaluator"};
            }
            
            _engine.createSession(args[0].c_str(), recipe);
            _engine.switchSession(args[0].c_str());
            std::cout << "Session '" << args[0] << "' created and activated.\n";
        });

        _dispatcher.registerCommand(":session use", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :session use <name>");
            _engine.switchSession(args[0].c_str());
        });

        _dispatcher.registerCommand(":session delete", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :session delete <name>");
            _engine.deleteSession(args[0].c_str());
            std::cout << "Session deleted.\n";
        });


        //  ================
        //      PLUGINS
        //  ================
        auto cmd_plugin_list = [this](const auto&) { _printPlugins(); };
        _dispatcher.registerCommand(":plugin list", cmd_plugin_list);
        _dispatcher.registerCommand(":plugin", cmd_plugin_list);

        _dispatcher.registerCommand(":plugin install", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :plugin install <path> [--autoload]");
            bool autoload = (args.size() > 1 && args[1] == "--autoload");
            _workspace.installPlugin(args[0], autoload);
            std::cout << GREEN << "Plugin installed successfully.\n" << RESET;
        });

        _dispatcher.registerCommand(":plugin uninstall", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :plugin uninstall <filename>");
            _workspace.uninstallPlugin(args[0]);
            std::cout << "Plugin uninstalled from workspace.\n";
        });

        _dispatcher.registerCommand(":plugin autoload", [this](const auto& args) {
            if (args.size() < 2) throw std::runtime_error("Usage: :plugin autoload <filename> <on|off>");
            if (args[1] == "on") _workspace.enableAutoload(args[0]);
            else _workspace.disableAutoload(args[0]);
            std::cout << "Autoload settings updated.\n";
        });

        _dispatcher.registerCommand(":plugin check", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :plugin check <path>");
            std::cout << "Running sandbox check...\n";
            auto res = _workspace.checkPlugin(args[0]);
            
            if (res == WorkspaceManager::PluginCheckResult::Passed) 
                std::cout << GREEN << "[OK] Safe to load.\n" << RESET;
            else if (res == WorkspaceManager::PluginCheckResult::LoadError) 
                std::cout << YELLOW << "[FAILED] Plugin did not crash, but failed to load (invalid ABI or missing symbols).\n" << RESET;
            else if (res == WorkspaceManager::PluginCheckResult::Crashed) 
                std::cout << RED << "[FATAL] Plugin crashed (Segfault/Timeout) in sandbox!\n" << RESET;
            else 
                std::cout << YELLOW << "[WARNING] Sandbox unavailable on this OS.\n" << RESET;
        });

        _dispatcher.registerCommand(":plugin load", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :plugin load <name>");
            
#ifdef _WIN32
            std::string ext = ".dll";
#else
            std::string ext = ".so";
#endif
            std::string full_path = (std::filesystem::path(_workspace.getPluginsDir()) / (args[0] + ext)).string();
            
            _workspace.markForTesting(args[0]);
            _engine.loadPluginFromFile(full_path.c_str());
            _workspace.clearTestingMark();
            std::cout << GREEN << "Plugin loaded into memory.\n" << RESET;
        });

        _dispatcher.registerCommand(":plugin manifest", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :plugin manifest <name>");
            auto m = _engine.getPluginManifest(args[0].c_str());
            std::cout << CYAN << "Manifest for " << args[0] << ":\n" << RESET;
            std::cout << "  Variables: " << m.variables.size() << "\n";
            std::cout << "  Functions: " << m.functions.size() << "\n";
            std::cout << "  Lexers:    " << m.lexers.size() << "\n";
            std::cout << "  Parsers:   " << m.parsers.size() << "\n";
            std::cout << "  Visitors:  " << m.visitors.size() << "\n";
        });


        //  ========================
        //      TOOLS n RECIPES
        //  ========================
        auto cmd_tool_list = [this](const auto&) { _printTools(); };
        _dispatcher.registerCommand(":tool list", cmd_tool_list);
        _dispatcher.registerCommand(":tool", cmd_tool_list);

        _dispatcher.registerCommand(":tool info", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :tool info <name>");
            std::string search = args[0];
            bool found = false;

            auto check = [&](const auto& tools, const char* type) {
                for (const auto& t : tools) {
                    if (t.getFullName() == search || t.tool_name == search) {
                        std::cout << CYAN << "[" << type << "] " << t.getFullName() << RESET << "\n";
                        std::cout << "  Description: " << t.description << "\n";
                        found = true;
                    }
                }
            };
            check(_engine.getAvailableLexers(), "Lexer");
            check(_engine.getAvailableParsers(), "Parser");
            check(_engine.getAvailableVisitors(), "Visitor");

            if (!found) std::cout << YELLOW << "Tool '" << search << "' not found.\n" << RESET;
        });

        auto cmd_recipe_list = [this](const auto&) {
            auto recipes = _workspace.getAvailableRecipes();
            if (recipes.empty()) std::cout << "No saved recipes.\n";
            for (const auto& r : recipes) std::cout << " - " << r << "\n";
        };
        _dispatcher.registerCommand(":recipe list", cmd_recipe_list);
        _dispatcher.registerCommand(":recipe", cmd_recipe_list);

        _dispatcher.registerCommand(":recipe save", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :recipe save <name>");
            _workspace.saveRecipe(args[0], _engine.getActiveSessionRecipe());
            std::cout << GREEN << "Recipe saved successfully.\n" << RESET;
        });

        _dispatcher.registerCommand(":recipe delete", [this](const auto& args) {
            if (args.empty()) throw std::runtime_error("Usage: :recipe delete <name>");
            _workspace.deleteRecipe(args[0]);
            std::cout << "Recipe deleted.\n";
        });

        _dispatcher.registerCommand(":format", [this](const auto& args) {
            if (args.size() < 2) throw std::runtime_error("Usage: :format <visitor_name> <expression>");
            std::string expr = args[1];
            for (size_t i = 2; i < args.size(); ++i) expr += " " + args[i];
            Value res = _engine.formatAST(args[0].c_str(), expr);
            std::cout << res.to_str() << "\n";
        });


        // ==========================================
        //  4. ПАМ'ЯТЬ ТА СИСТЕМА
        // ==========================================
        _dispatcher.registerCommand(":env", [this](const auto&) { _printEnvironment(); });
        
        _dispatcher.registerCommand(":history", [this](const auto&) {
            if (_history.empty()) return;
            for (size_t i = 0; i < _history.size(); ++i) {
                std::cout << std::setw(3) << i + 1 << "  " << _history[i] << "\n";
            }
        });

        _dispatcher.registerCommand(":clear", [](const auto&) {
            std::cout << "\033[2J\033[1;1H"; // ANSI Escape для очищення екрану
        });

        _dispatcher.registerCommand(":workspace reset", [this](const auto&) {
            std::cout << RED << "Are you sure you want to reset workspace? All plugins and recipes will be deleted! (y/N): " << RESET;
            std::string ans;
            std::getline(std::cin, ans);
            if (ans == "y" || ans == "Y") {
                _workspace.resetWorkspace();
                std::cout << YELLOW << "Workspace reset to factory defaults.\n" << RESET;
            } else {
                std::cout << "Aborted.\n";
            }
        });

        _dispatcher.registerCommand(":help", [this](const auto&) { _printHelp(); });
    }

    void ReplApp::Impl::_printHelp() const {
        std::cout << "\n" << CYAN << "--- Session Management ---" << RESET << "\n"
                  << "  :session [list]                       " << YELLOW << "- Show available sessions" << RESET << "\n"
                  << "  :session new <name> [recipe]          " << YELLOW << "- Create a new calculation session" << RESET << "\n"
                  << "  :session use <name>                   " << YELLOW << "- Switch to an existing session" << RESET << "\n"
                  << "  :session delete <name>                " << YELLOW << "- Delete a session" << RESET << "\n"
                  
                  << "\n" << CYAN << "--- Plugin & Workspace Control ---" << RESET << "\n"
                  << "  :plugin [list]                        " << YELLOW << "- Show plugin statuses (loaded, autoload, quarantined)" << RESET << "\n"
                  << "  :plugin install <path> [--autoload]   " << YELLOW << "- Copy plugin to workspace" << RESET << "\n"
                  << "  :plugin uninstall <name>              " << YELLOW << "- Remove plugin from workspace" << RESET << "\n"
                  << "  :plugin check <path>                  " << YELLOW << "- Safely test plugin in a sandbox" << RESET << "\n"
                  << "  :plugin autoload <name> <on|off>      " << YELLOW << "- Toggle autoload on startup" << RESET << "\n"
                  << "  :plugin load <name>                   " << YELLOW << "- Load an installed plugin into memory" << RESET << "\n"
                  << "  :plugin manifest <name>               " << YELLOW << "- Show exposed tools and functions" << RESET << "\n"

                  << "\n" << CYAN << "--- Tools & Recipes ---" << RESET << "\n"
                  << "  :tool [list]                          " << YELLOW << "- Show available lexers, parsers, visitors" << RESET << "\n"
                  << "  :tool info <name>                     " << YELLOW << "- Show detailed info about a specific tool" << RESET << "\n"
                  << "  :recipe [list]                        " << YELLOW << "- Show saved configurations" << RESET << "\n"
                  << "  :recipe save <name>                   " << YELLOW << "- Save current session config as a recipe" << RESET << "\n"
                  << "  :recipe delete <name>                 " << YELLOW << "- Delete a saved recipe" << RESET << "\n"
                  << "  :format <visitor_name> <expression>   " << YELLOW << "- Pass expression through a specific visitor" << RESET << "\n"

                  << "\n" << CYAN << "--- System ---" << RESET << "\n"
                  << "  :env                                  " << YELLOW << "- Show memory (variables & functions) for active session" << RESET << "\n"
                  << "  :history                              " << YELLOW << "- Show command history" << RESET << "\n"
                  << "  :clear                                " << YELLOW << "- Clear terminal screen" << RESET << "\n"
                  << "  :workspace reset                      " << YELLOW << "- Factory reset (DELETES ALL PLUGINS AND RECIPES)" << RESET << "\n"
                  << "  :help, :quit                          " << YELLOW << "- Show this menu or exit" << RESET << "\n\n";
    }

    void ReplApp::Impl::_printPlugins() const {
        auto autoloaded = _workspace.getAutoloadedPlugins();
        std::vector<std::string> auto_names;
        for (const auto& p : autoloaded) {
            auto_names.push_back(std::filesystem::path(p).stem().string());
        }

        auto quarantined_names = _workspace.getQuarantinedPlugins();

        auto loaded_plugins = _engine.getLoadedPlugins();
        std::vector<std::string> memory_names;
        for (const auto& info : loaded_plugins) {
            memory_names.push_back(info.name);
        }

        std::cout << "Workspace plugins (~/.synapse/plugins/):\n";
        
        for (const auto& name : _workspace.getInstalledPlugins()) {
            bool is_auto   = (std::find(auto_names.begin(), auto_names.end(), name) != auto_names.end());
            bool is_quar   = (std::find(quarantined_names.begin(), quarantined_names.end(), name) != quarantined_names.end());
            bool is_loaded = (std::find(memory_names.begin(), memory_names.end(), name) != memory_names.end());
            
            std::cout << "  ";

            //  mem state
            if (is_loaded) std::cout << CYAN << "[L]" << RESET;
            else std::cout << "[ ]";

            // file state
            if (is_quar) {
                std::cout << RED << "[Q] " << name << " (Quarantined - Crashed recently)\n" << RESET;
            } else if (is_auto) {
                std::cout << GREEN << "[A] " << name << " (Autoloaded)\n" << RESET;
            } else {
                std::cout << "[I] " << name << "\n";
            }
        }

        std::cout << "\nBuilt-in plugins in memory:\n";
        for (const auto& name : memory_names) {
            auto installed = _workspace.getInstalledPlugins();
            if (std::find(installed.begin(), installed.end(), name) == installed.end()) {
                std::cout << CYAN << "  [L]     " << name << "\n" << RESET;
            }
        }
    }

    void ReplApp::Impl::_printTools() const {
        auto print_group = [](const char* title, const auto& tools) {
            std::cout << CYAN << title << RESET << "\n";
            for (const auto& t : tools) std::cout << "  - " << t.getFullName() << "\n";
        };
        print_group("--- Lexers ---", _engine.getAvailableLexers());
        print_group("\n--- Parsers ---", _engine.getAvailableParsers());
        print_group("\n--- Visitors ---", _engine.getAvailableVisitors());
    }

    void ReplApp::Impl::_printEnvironment() const {
        std::cout << CYAN << "--- Variables ---\n" << RESET;
        _engine.enumerateActiveSessionVariables([](const auto& v) {
            std::cout << std::left << std::setw(15) << v.name 
                      << std::setw(15) << v.value.to_str() 
                      << MAGENTA << "[" << v.source << "]\n" << RESET;
        });
        std::cout << CYAN << "\n--- Functions ---\n" << RESET;
        _engine.enumerateActiveSessionFunctions([](const auto& f) {
            std::cout << std::left << std::setw(15) << f.name 
                      << "Arity: " << std::setw(5) << (f.arity == ICallable::VARITY ? "VAR" : std::to_string(f.arity)) 
                      << MAGENTA << "[" << f.source << "]\n" << RESET;
        });
    }

}   //  namespace   Synapse::Repl
