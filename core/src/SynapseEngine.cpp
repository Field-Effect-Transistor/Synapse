// /core/src/SynapseEngine.cpp
#include "synapse/SynapseEngine.hpp"

#include "synapse/ExecutionContext.hpp"
#include "synapse/EnvironmentManager.hpp"
#include "synapse/Module.hpp"

#include <string>
#include <stdexcept>
#include <unordered_map>
#include "Vector.hpp"

namespace Synapse {

    struct SynapseEngine::Impl {
        struct Session {
            std::string         name;
            Calculator::Recipe  recipe;
            ExecutionContext*   context = nullptr;
        };

        std::unordered_map<std::string, Session> _sessions;
        Session*    _active_session = nullptr;
        
        PluginRegistry      _plugin_registry;
        EnvironmentManager  _env_manager;
    };
    
    SynapseEngine::SynapseEngine() : _impl(new Impl) {};
    SynapseEngine::~SynapseEngine() = default;
    SynapseEngine::SynapseEngine(SynapseEngine&&) = default;
    SynapseEngine& SynapseEngine::operator=(SynapseEngine&&) = default;

    //  ----------------
    //      PLUGINS
    //  ----------------

    void SynapseEngine::loadPlugin(IPlugin::Ptr&& plugin) {
        std::string name = plugin->getName();
        
        _impl->_plugin_registry.loadPlugin(std::move(plugin));
        
        Module* mod = _impl->_env_manager.createPluginModule(name.c_str());
        _impl->_plugin_registry.fillModule(*mod, name.c_str());
    }

    void SynapseEngine::loadPluginFromFile(const char* path) {
        size_t plugins_before = _impl->_plugin_registry.getLoadedPlugins().size();
        _impl->_plugin_registry.loadFromFile(path);
        
        auto plugins = _impl->_plugin_registry.getLoadedPlugins();
        for (size_t i = plugins_before; i < plugins.size(); ++i) {
            std::string name = plugins[i].name;
            Module* mod = _impl->_env_manager.createPluginModule(name.c_str());
            _impl->_plugin_registry.fillModule(*mod, name.c_str());
        }
    }

    void SynapseEngine::defineGlobalVariable(const char* name, Value val, bool is_const) {
        _impl->_env_manager.getGlobalModule()->getTable().defineVariable(name, std::move(val), is_const);
    }

    void SynapseEngine::defineGlobalFunction(const char* name, ICallable::Ptr&& func) {
        _impl->_env_manager.getGlobalModule()->getTable().defineFunction(name, std::move(func));
    }

    //  --------------------
    //      DISCOVERY API
    //  --------------------

    Vector<PluginInfo> SynapseEngine::getLoadedPlugins() const {
        return _impl->_plugin_registry.getLoadedPlugins();
    }

    Vector<ToolInfo>   SynapseEngine::getAvailableLexers() const {
        return _impl->_plugin_registry.getAvailableLexers();
    }

    Vector<ToolInfo>   SynapseEngine::getAvailableParsers() const {
        return _impl->_plugin_registry.getAvailableParsers();
    }

    Vector<ToolInfo>   SynapseEngine::getAvailableVisitors() const {
        return _impl->_plugin_registry.getAvailableVisitors();
    }


    //  ------------------------
    //      SESSION CONTROL
    //  ------------------------

    void SynapseEngine::createSession(const char* session_name, const Calculator::Recipe& recipe) {
        std::string name(session_name);
        if (_impl->_sessions.count(name)) {
            throw std::runtime_error("Session '" + name + "' already exists!");
        }

        // 1. Делегуємо створення сесії Менеджеру
        ExecutionContext* session_ctx = _impl->_env_manager.createSessionContext();
        
        // 2. Підключаємо Глобальний модуль
        session_ctx->importModule(_impl->_env_manager.getGlobalModule());
        
        // 3. Підключаємо всі завантажені плагіни
        auto plugins = _impl->_plugin_registry.getLoadedPlugins();
        for (const auto& p : plugins) {
            session_ctx->importModule(_impl->_env_manager.getPluginModule(p.name.c_str()));
        }

        // 4. Зберігаємо сесію (тільки сирий вказівник)
        _impl->_sessions[name] = Impl::Session{name, recipe, session_ctx};

        if (!_impl->_active_session) {
            _impl->_active_session = &_impl->_sessions[name];
        }
    }

    void SynapseEngine::switchSession(const char* session_name) {
        std::string name(session_name);
        auto it = _impl->_sessions.find(name);
        if (it == _impl->_sessions.end()) {
            throw std::runtime_error("Session '" + name + "' not found!");
        }
        _impl->_active_session = &(it->second);
    }

    bool SynapseEngine::hasSession(const char* session_name) const {
        return _impl->_sessions.count(session_name) > 0;
    }

    void SynapseEngine::deleteSession(const char* session_name) {
        std::string name(session_name);
        auto it = _impl->_sessions.find(name);
        if (it == _impl->_sessions.end()) {
            throw std::runtime_error("Session '" + name + "' not found!");
        }

        if (_impl->_active_session == &(it->second)) {
            _impl->_active_session = nullptr;
        }

        _impl->_env_manager.deleteSessionContext(it->second.context);
        
        _impl->_sessions.erase(it);
    }

    const char* SynapseEngine::getActiveSessionName() const {
        if (!_impl->_active_session) return "";
        return _impl->_active_session->name.c_str();
    }

    Vector<std::string> SynapseEngine::getAvailableSessions() const {
        Vector<std::string> names;
        for (const auto& pair : _impl->_sessions) {
            names.push_back(pair.first);
        }
        return names;
    }


    //  --------------------
    //      EXECUTION
    //  --------------------

    Value SynapseEngine::evaluate(const std::string& code) {
        if (!_impl->_active_session) {
            throw std::runtime_error("No active session! Create and switch to a session first.");
        }

        Calculator calc(&_impl->_plugin_registry, _impl->_active_session->recipe);

        Value result = calc.evaluate(code, _impl->_active_session->context);

       if (_impl->_active_session->context->hasLocalVariable("ans")) {
            _impl->_active_session->context->assignVariable("ans", result);
        } else {
            _impl->_active_session->context->defineVariable("ans", result);
        }

        return result;
    }

    Value SynapseEngine::formatAST(const char* printer_visitor_name, const std::string& code) {
        if (!_impl->_active_session) {
            throw std::runtime_error("No active session!");
        }

        Calculator::Recipe print_recipe = _impl->_active_session->recipe;
        print_recipe.evaluator = printer_visitor_name;
        
        print_recipe.optimizers.clear();

        Calculator calc(&_impl->_plugin_registry, print_recipe);

        return calc.evaluate(code, _impl->_active_session->context);
    }
    
}   //  namespace   Synapse
