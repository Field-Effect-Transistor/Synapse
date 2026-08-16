// /core/include/synapse/Context.hpp
#pragma once

#include "Value.hpp"
#include "UniquePtr.hpp"
#include "interface/ICallable.hpp"

namespace Synapse {
    class Context final {
    private:
        struct Impl;
        
        UniquePtr<Impl> _impl;
        Context*        _parent;

        explicit Context(Context* parent = nullptr);
        ~Context();

    public:
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        Context(Context&&) noexcept;
        Context& operator=(Context&&) noexcept;

        void defineVariable(const char* name, Value val, bool is_const = false);
        void assignVariable(const char* name, Value val);
        Value getVariable(const char* name) const;

        void defineFunction(const char* name, ICallable::Ptr function);
        ICallable* getFunction(const char* name) const;

        friend class ContextManager;
        friend class ContextTestWrapper;
    };

} // namespace Synapse
