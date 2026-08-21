//  /core/src/DynamicLibrary.cpp
#include "synapse/DynamicLibrary.hpp"

#include "synapse/PluginRegistry.hpp"
#include "synapse/interface/IPlugin.hpp"

#include <cassert>
#include <dlfcn.h>
#include <stdexcept>

typedef void* handler_t;

namespace Synapse {

    struct DynamicLibrary::Impl {
        handler_t _handler = nullptr;
    };  //  struct DynamicLibrary::Impl

    DynamicLibrary::DynamicLibrary(const char* path) : _impl(new Impl) {
        if (!path) {
            throw std::runtime_error("[DynamicLibrary] path can not be nullptr");
        }

        dlerror();

        handler_t handler = dlopen(path, RTLD_LAZY);
        if (!handler) {
            const char* err = dlerror();
            throw std::runtime_error(
                "[DynamicLibrary] dlopen crash with next error: " +
                std::string(err ? err : "unknown dlopen error"));
        }

        _impl->_handler = handler;
    }

    DynamicLibrary::~DynamicLibrary() {
        assert(_impl && "DynamicLibrary::_impl must never be null");

        if (_impl->_handler) {
            dlclose(_impl->_handler);
        }
    }

    DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept : _impl(new Impl) {
        assert(other._impl && "moved-from DynamicLibrary must have valid _impl");

        _impl->_handler = other._impl->_handler;
        other._impl->_handler = nullptr;
    }

    DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        assert(_impl && other._impl && "both operands must have valid _impl");

        std::swap(this->_impl->_handler, other._impl->_handler);
        return *this;
    }

    IPlugin::Ptr DynamicLibrary::getSymbol() {
        assert(_impl && "DynamicLibrary::_impl must never be null");

        if (!_impl->_handler) {
            throw std::runtime_error(
                "[DynamicLibrary] library is not loaded (moved-from or already closed)");
        }

        using CreatePluginFn = IPlugin* (*)();

        dlerror();
        void* symbol = dlsym(_impl->_handler, "create_plugin");

        const char* raw_err = dlerror();
        const std::string err_msg = raw_err ? raw_err : "";

        if (!symbol || !err_msg.empty()) {
            if (_impl->_handler) {
                dlclose(_impl->_handler);
                _impl->_handler = nullptr;
            }
            throw std::runtime_error(
                "[DynamicLibrary] dlsym error: " +
                (err_msg.empty() ? std::string("symbol 'create_plugin' not found") : err_msg));
        }

        auto factory = reinterpret_cast<CreatePluginFn>(symbol);
        IPlugin* plugin = factory();

        if (!plugin) {
            throw std::runtime_error("[DynamicLibrary] create_plugin() returned nullptr");
        }

        return IPlugin::Ptr(plugin);
    }

}   //  namespace   Synapse
