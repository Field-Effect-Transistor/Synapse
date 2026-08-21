//  /core/src/EnvironmentManager.cpp
#include "synapse/EnvironmentManager.hpp"

#include "synapse/ExecutionContext.hpp"
#include "synapse/Module.hpp"
#include "Vector.hpp"

#include <unordered_map>
#include <string>

namespace Synapse {

    struct EnvironmentManager::Impl {
        UniquePtr<Module> _global_module;
        std::unordered_map<std::string, UniquePtr<Module>> _plugin_modules;
        Vector<UniquePtr<ExecutionContext>> _sessions;

        Impl() : _global_module(new Module("Global")) {}

    };  //  struct  EnvironmentManager::Impl

    //  --------------------
    //      Lifecycle
    //  --------------------

    EnvironmentManager::EnvironmentManager() : _impl(new Impl()) {}
    
    EnvironmentManager::~EnvironmentManager() = default; 

    EnvironmentManager::EnvironmentManager(EnvironmentManager&&) noexcept = default;
    EnvironmentManager& EnvironmentManager::operator=(EnvironmentManager&&) noexcept = default;

    //  --------------------
    //      Global Memory
    //  --------------------

    Module* EnvironmentManager::getGlobalModule() const {
        return _impl->_global_module.get();
    }

    //  --------------------
    //      Plugin Memory
    //  --------------------

    Module* EnvironmentManager::createPluginModule(const char* name) {
        std::string key(name);
        _impl->_plugin_modules[key] = UniquePtr<Module>(new Module(name));
        return _impl->_plugin_modules[key].get();
    }

    Module* EnvironmentManager::getPluginModule(const char* name) const {
        auto it = _impl->_plugin_modules.find(name);
        if (it != _impl->_plugin_modules.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    //  ------------------------
    //      Session Memory
    //  ------------------------

    ExecutionContext* EnvironmentManager::createSessionContext() {
        auto ctx = UniquePtr<ExecutionContext>(new ExecutionContext());
        ExecutionContext* raw_ptr = ctx.get();
        
        _impl->_sessions.push_back(std::move(ctx));
        
        return raw_ptr;
    }

    void EnvironmentManager::deleteSessionContext(ExecutionContext* target) {
        for (auto it = _impl->_sessions.begin(); it != _impl->_sessions.end(); ++it) {
            if (it->get() == target) {
                _impl->_sessions.erase(it);
                return;
            }
        }
    }

}   //  namespace   Synapse
