//  /core/include/synapse/PluginRegistry.hpp
#pragma once

#include "synapse/interface/IPlugin.hpp"
#include "synapse/interface/ILexer.hpp"
#include "synapse/interface/IParser.hpp"
#include "synapse/interface/IVisitor.hpp"

#include "Vector.hpp"
#include <string>

namespace Synapse {

    class ExecutionContext;
    class Module;

    //  DISCOVERY API STRUCTURES
    struct PluginInfo {
        std::string name;
        std::string version;
        std::string description;
    };

    struct ToolInfo {
        std::string plugin_name;
        std::string tool_name;
        std::string description;
        
        std::string getFullName() const { return plugin_name + "." + tool_name; }
    };

    class PluginRegistry final {
    private:
        struct Impl;
        UniquePtr<Impl> _impl;

    public:
        PluginRegistry();
        ~PluginRegistry();

        PluginRegistry(const PluginRegistry&) = delete;
        PluginRegistry& operator=(const PluginRegistry&) = delete;

        PluginRegistry(PluginRegistry&&) noexcept;
        PluginRegistry& operator=(PluginRegistry&&) noexcept;

        //  DYNAMIC LIBRARY
        void loadFromFile(const char* name);

        //  LIFECYCLE
        void loadPlugin(IPlugin::Ptr plugin);
        void fillModule(Module& mod, const char* plugin_name) const;

        //  DISCOVERY API
        Vector<PluginInfo> getLoadedPlugins() const;
        Vector<ToolInfo>   getAvailableLexers() const;
        Vector<ToolInfo>   getAvailableParsers() const;
        Vector<ToolInfo>   getAvailableSimpleVisitors() const;
        Vector<ToolInfo>   getAvailableContextualVisitors() const;

        //  FACTORY API (EXACT)
        ILexer::Ptr     createLexer(const char* plugin_name, const char* lexer_name) const;
        IParser::Ptr    createParser(const char* plugin_name, const char* parser_name) const;
        IVisitor::Ptr   createSimpleVisitor(const char* plugin_name, const char* visitor_name) const;
        IVisitor::Ptr   createContextualVisitor(const char* plugin_name, const char* visitor_name, ExecutionContext* ctx) const;

        //  FACTORY API (SMART)
        ILexer::Ptr     createLexer(const char* name) const;
        IParser::Ptr    createParser(const char* name) const;
        IVisitor::Ptr   createSimpleVisitor(const char* name) const;
        IVisitor::Ptr   createContextualVisitor(const char* name, ExecutionContext* ctx) const;

        //  VALIDATION API
        bool hasLexer(const char* plugin_name, const char* lexer_name) const;
        bool hasParser(const char* plugin_name, const char* parser_name) const;
        bool hasSimpleVisitor(const char* plugin_name, const char* visitor_name) const;
        bool hasContextualVisitor(const char* plugin_name, const char* visitor_name) const;

        bool hasLexer(const char* name) const;
        bool hasParser(const char* name) const;
        bool hasSimpleVisitor(const char* name) const;
        bool hasContextualVisitor(const char* name) const;

    };  //  class   PluginRegistry
}   //  namespace   Synapse
