//  /core/src/DynamicLibrary.cpp
#include "synapse/DynamicLibrary.hpp"

#include "synapse/PluginRegistry.hpp"
#include "synapse/interface/IPlugin.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

//  ====================
//      OS API LAYER
//  ====================
#ifdef _WIN32
    #include <windows.h>
    typedef HMODULE os_handle_t;

    namespace OS {
        inline os_handle_t load_library(const char* path) { return LoadLibraryA(path); }
        inline void free_library(os_handle_t handle) { FreeLibrary(handle); }
        inline void* get_symbol(os_handle_t handle, const char* name) { return (void*)GetProcAddress(handle, name); }
        
        inline std::string get_last_error() {
            return "Windows Error Code: " + std::to_string(GetLastError());
        }
        inline void reset_error() {}
    }
#else
    #include <dlfcn.h>
    typedef void* os_handle_t;

    namespace OS {
        inline os_handle_t load_library(const char* path) { return dlopen(path, RTLD_LAZY); }
        inline void free_library(os_handle_t handle) { dlclose(handle); }
        inline void* get_symbol(os_handle_t handle, const char* name) { return dlsym(handle, name); }
        
        inline std::string get_last_error() {
            const char* err = dlerror();
            return err ? std::string(err) : "Unknown OS error";
        }
        inline void reset_error() { dlerror(); }
    }
#endif
// ==========================================


namespace Synapse {

    struct DynamicLibrary::Impl {
        os_handle_t _handler = nullptr;
    };

    DynamicLibrary::DynamicLibrary(const char* path) : _impl(new Impl) {
        if (!path) {
            throw std::runtime_error("[DynamicLibrary] path can not be nullptr");
        }

        OS::reset_error();
        os_handle_t handler = OS::load_library(path);
        
        if (!handler) {
            throw std::runtime_error("[DynamicLibrary] Library load crash: " + OS::get_last_error());
        }

        _impl->_handler = handler;
    }

    DynamicLibrary::~DynamicLibrary() {
        assert(_impl && "DynamicLibrary::_impl must never be null");

        if (_impl->_handler) {
            OS::free_library(_impl->_handler);
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
            throw std::runtime_error("[DynamicLibrary] library is not loaded (moved-from or already closed)");
        }

        using CreatePluginFn = IPlugin* (*)();

        OS::reset_error();
        void* symbol = OS::get_symbol(_impl->_handler, "create_plugin");

        if (!symbol) {
            std::string err_msg = OS::get_last_error();
            OS::free_library(_impl->_handler);
            _impl->_handler = nullptr;
            throw std::runtime_error("[DynamicLibrary] Symbol 'create_plugin' not found: " + err_msg);
        }

        auto factory = reinterpret_cast<CreatePluginFn>(symbol);
        IPlugin* plugin = factory();

        if (!plugin) {
            throw std::runtime_error("[DynamicLibrary] create_plugin() returned nullptr");
        }

        return IPlugin::Ptr(plugin);
    }
}   //  namespace   Synapse
