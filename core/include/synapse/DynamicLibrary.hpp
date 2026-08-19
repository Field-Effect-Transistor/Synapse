//  /core/include/synapse/DynamicLibrary.hpp
#pragma once

#include "synapse/interface/IPlugin.hpp"

#include "UniquePtr.hpp"
#include "Vector.hpp"

namespace Synapse {

    class PluginRegistry;   //  FWD_DECL

    class DynamicLibrary final {
        struct Impl;
        UniquePtr<Impl> _impl;

    public:
        explicit DynamicLibrary(const char* path);
        ~DynamicLibrary();

        DynamicLibrary(const DynamicLibrary&) = delete;
        DynamicLibrary& operator=(const DynamicLibrary&) = delete;

        DynamicLibrary(DynamicLibrary&&) noexcept;
        DynamicLibrary& operator=(DynamicLibrary&&) noexcept;

        IPlugin::Ptr getSymbol();

    };  //  class   DynamicLibrary

}   //  namespace   Synapse
