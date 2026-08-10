//  /core/include/synapse/ContextManager.hpp
#pragma once

//#include "synapse/Context.hpp"

#include "UniquePtr.hpp"
#include "Vector.hpp"

namespace Synapse {
    class Context;

    class ContextManager final {
        static void deleteContext(Context* ptr);
        struct ContextDeleter { void operator()(Context* ptr) const; };
        using ContextPtr = UniquePtr<Context, ContextDeleter>;

        Vector<ContextPtr>  _contexts;
        Context*            _global_scope = nullptr;

    public:
        ContextManager();
        ~ContextManager() = default;

        ContextManager(ContextManager&&) noexcept;
        ContextManager& operator=(ContextManager&&) noexcept;

        ContextManager(const ContextManager&) = delete;
        ContextManager& operator=(const ContextManager&) = delete;

        Context* createScope(Context* parent = nullptr);
        inline Context* getGlobalScope() const { return _global_scope; }
        ContextManager& clear();

    };  //  class   ContextManager

}   //  namespace   Synapse
