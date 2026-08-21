//  /core/include/synapse/SynapseEngine.hpp
#pragma once

#include "synapse/Calculator.hpp"
#include "synapse/PluginRegistry.hpp"

#include "synapse/interface/IPlugin.hpp"
#include "synapse/interface/ICallable.hpp"

namespace Synapse {

    class SynapseEngine final {
        struct Impl;
        UniquePtr<Impl> _impl;

    public:
        SynapseEngine();
        ~SynapseEngine();

        SynapseEngine(const SynapseEngine&) = delete;
        SynapseEngine& operator=(const SynapseEngine&) = delete;

        SynapseEngine(SynapseEngine&&);
        SynapseEngine& operator=(SynapseEngine&&);

        //  ----------------
        //      PLUGINS
        //  ----------------

        void loadPlugin(IPlugin::Ptr&& plugin);
        void loadPluginFromFile(const char* path);

        void defineGlobalVariable(const char* name, Value val, bool is_const = false);
        void defineGlobalFunction(const char* name, ICallable::Ptr&& func);


        //  --------------------
        //      DISCOVERY API
        //  --------------------

        Vector<PluginInfo> getLoadedPlugins() const;
        Vector<ToolInfo>   getAvailableLexers() const;
        Vector<ToolInfo>   getAvailableParsers() const;
        Vector<ToolInfo>   getAvailableVisitors() const;


        //  ------------------------
        //      SESSION CONTROL
        //  ------------------------

        void createSession(const char* session_name, const Calculator::Recipe& recipe);
        void switchSession(const char* session_name);
        bool hasSession(const char* session_name) const;
        void deleteSession(const char* session_name);

        const char* getActiveSessionName() const;
        Vector<std::string> getAvailableSessions() const;


        //  ----------------
        //      EXECUTION
        //  ----------------

        Value evaluate(const std::string& code);
        Value formatAST(const char* printer_visitor_name, const std::string& code);

    }; 

}   //  namespace   Synapse
