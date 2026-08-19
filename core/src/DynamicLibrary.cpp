//  /core/src/DynamicLibrary.cpp
#include "synapse/DynamicLibrary.hpp"

#include "synapse/PluginRegistry.hpp"

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

        handler_t handler = dlopen(path, RTLD_LAZY);
        if(!handler) {
            throw std::runtime_error("[DynamicLibrary] dlopen crash with next error: " + std::string(dlerror()));
        }
        dlerror();
    }

    DynamicLibrary::~DynamicLibrary() {
        if (_impl->_handler) {
            dlclose(_impl->_handler);
        }
    }

    DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept {
        this->_impl->_handler = other._impl->_handler;
        other._impl->_handler = nullptr;
    }

    DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept {
        std::swap(this->_impl->_handler, other._impl->_handler);
        return *this;
    }

    IPlugin::Ptr DynamicLibrary::getSymbol() {
        void* create_plugin = nullptr;
        if (_impl->_handler) {
            create_plugin = dlsym(_impl->_handler, "create_plugin");
        }

        if (create_plugin) {
            return IPlugin::Ptr(static_cast<IPlugin*>(create_plugin));
        }

        dlclose(_impl->_handler);
        _impl->_handler = 0;
        throw std::runtime_error("[DynamicLibrary] dlsym error: " + std::string(dlerror()));
    }

}   //  namespace   Synapse