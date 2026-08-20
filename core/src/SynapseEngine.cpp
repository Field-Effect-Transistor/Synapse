//  /core/src/SynapseEngine
#include "synapse/SynapseEngine.hpp"

#include "synapse/ContextManager.hpp"
#include "synapse/Context.hpp"

#include <string>
#include <Vector.hpp>
#include <unordered_map>

namespace Synapse {

    struct SynapseEngine::Impl {

        struct Session {
            std::string         name;
            Calculator::Recipe  recipe;
            Context*            context = nullptr;
        };  //  struct Session

        std::unordered_map<std::string, Session> _sessions;
        Session*    _active_session = nullptr;
        
        PluginRegistry  _plugin_registry;
        ContextManager  _context_manager;

    };  //  struct  SynapseEngine::Impl
    
    SynapseEngine::SynapseEngine() : _impl(new Impl) {};
    SynapseEngine::~SynapseEngine() = default;

    SynapseEngine::SynapseEngine(SynapseEngine&&) = default;
    SynapseEngine& SynapseEngine::operator=(SynapseEngine&&) = default;


    //  ----------------
    //      PLUGINS
    //  ----------------

    void SynapseEngine::loadPlugin(IPlugin::Ptr&& plugin) {
        _impl->_plugin_registry.loadPlugin(std::move(plugin));
    }

    void SynapseEngine::loadPluginFromFile(const char* path) {
        _impl->_plugin_registry.loadFromFile(path);
    }

    void SynapseEngine::defineGlobalVariable(const char* name, Value val, bool is_const) {
        _impl->_context_manager.getGlobalScope()->defineVariable(name, val, is_const);
    }

    void SynapseEngine::defineGlobalFunction(const char* name, ICallable::Ptr&& func) {
        _impl->_context_manager.getGlobalScope()->defineFunction(name, std::move(func));
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

    Vector<ToolInfo>   SynapseEngine::getAvailableSimpleVisitors() const {
        return _impl->_plugin_registry.getAvailableSimpleVisitors();
    }

    Vector<ToolInfo>   SynapseEngine::getAvailableContextualVisitors() const {
        return _impl->_plugin_registry.getAvailableContextualVisitors();
    }


    //  ------------------------
    //      SESSION CONTROL
    //  ------------------------

    void SynapseEngine::createSession(const char* session_name, const Calculator::Recipe& recipe) {
        std::string name(session_name);
        if (_impl->_sessions.count(name)) {
            throw std::runtime_error("Session '" + name + "' already exists!");
        }

        Context* session_ctx = _impl->_context_manager.createScope(_impl->_context_manager.getGlobalScope());

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

        try {
            _impl->_active_session->context->assignVariable("ans", result);
        } catch (...) {
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

