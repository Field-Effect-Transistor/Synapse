//  /core/src/ContextManager.cpp
#include "synapse/ContextManager.hpp"

#include "synapse/Context.hpp"

namespace Synapse {
    void ContextManager::deleteContext(Context* ptr) {
        delete ptr;
    }

    void ContextManager::ContextDeleter::operator()(Context* ptr) const {
        deleteContext(ptr);
    }

    ContextManager::ContextManager() {
        _global_scope = createScope();
    }

    ContextManager::ContextManager(ContextManager&& other) noexcept 
        : _contexts(std::move(other._contexts)), 
        _global_scope(other._global_scope) 
    {
        other._global_scope = nullptr;
    }

    ContextManager& ContextManager::operator=(ContextManager&& other) noexcept {
        if (this != &other) {
            _contexts = std::move(other._contexts);
            _global_scope = other._global_scope;
            
            other._global_scope = nullptr;
        }
        return *this;
    }

    Context* ContextManager::createScope(Context* parent) {
        auto child = ContextPtr(new Context(parent));
        Context* to_return = child.get();
        _contexts.push_back(std::move(child));

        return to_return;
    }

    ContextManager& ContextManager::clear() {
        _contexts.clear();
        _global_scope = createScope(nullptr);

        return *this;
    }

};  //  namespace   Synapse
