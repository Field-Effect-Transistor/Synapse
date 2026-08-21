//  /core/include/synapse/EnvironmentManager.hpp
#pragma once

#include "UniquePtr.hpp"

namespace Synapse {

    class Module;
    class ExecutionContext;

    class EnvironmentManager final {
    private:
        struct Impl;
        UniquePtr<Impl> _impl;

    public:
        EnvironmentManager();
        ~EnvironmentManager(); 

        EnvironmentManager(const EnvironmentManager&) = delete;
        EnvironmentManager& operator=(const EnvironmentManager&) = delete;

        EnvironmentManager(EnvironmentManager&&) noexcept;
        EnvironmentManager& operator=(EnvironmentManager&&) noexcept;

        Module* getGlobalModule() const;

        Module* createPluginModule(const char* name);
        Module* getPluginModule(const char* name) const;

        ExecutionContext* createSessionContext();
        void deleteSessionContext(ExecutionContext* target);

    };  //  class   EnvironmentManager

}   //  namespace   Synapse
