//  /core/src/PluginRegistry.cpp
#include "synapse/PluginRegistry.hpp"

#include "synapse/DynamicLibrary.hpp"
#include "synapse/ExecutionContext.hpp"
#include "synapse/Module.hpp"

#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <string>

namespace Synapse {

    struct PluginRegistry::Impl {
        
        template<typename FactoryType>
        class FactoryRegistry {
            struct FactoryEntry {
                const char* plugin_name = nullptr;
                const char* tool_name   = nullptr;
                const char* description = nullptr;
                FactoryType factory     = nullptr;

                friend bool operator<(const FactoryEntry& first, const FactoryEntry& second) {
                    int plugin_cmp = std::strcmp(first.plugin_name, second.plugin_name);
                    if (plugin_cmp != 0) return plugin_cmp < 0;
                    return std::strcmp(first.tool_name, second.tool_name) < 0;
                }
            };

            Vector<FactoryEntry> _entries;

        public:
            Vector<FactoryEntry> find_with_plugin(const char* name) const;
            Vector<FactoryEntry> find_with_tool(const char* name) const;
            FactoryEntry         find(const char* plugin_name, const char* tool_name) const;
            FactoryEntry         findSmart(const char* name) const;

            bool has(const char* plugin_name, const char* tool_name) const;
            bool hasSmart(const char* name) const;

            FactoryRegistry& push(const char* plugin_name, const char* tool_name, const char* desc, FactoryType factory);
            
            Vector<ToolInfo> getAvailableTools() const;
        };

        //  CONTAINERS
        Vector<DynamicLibrary>  _libraries;
        Vector<IPlugin::Ptr>    _plugins;
        Vector<PluginInfo>      _plugin_infos;

        std::unordered_map<std::string, PluginManifest> _manifests;

        FactoryRegistry<PluginManifest::LexerFactory>   _lexers;
        FactoryRegistry<PluginManifest::ParserFactory>  _parsers;
        FactoryRegistry<PluginManifest::VisitorFactory> _visitors;
    };

    //  ----------------------------
    //      FACTORY REGISTRY
    //  ----------------------------

    template<typename FactoryType>
    auto PluginRegistry::Impl::FactoryRegistry<FactoryType>::find_with_plugin(const char* name) const -> Vector<FactoryEntry> {
        Vector<FactoryEntry> result;
        if (_entries.empty()) return result;

        FactoryEntry target { name, "", nullptr, nullptr };
        auto it = std::lower_bound(_entries.begin(), _entries.end(), target);

        while (it != _entries.end() && std::strcmp(it->plugin_name, name) == 0) {
            result.push_back(*it);
            ++it;
        }
        return result;
    }

    template<typename FactoryType>
    auto PluginRegistry::Impl::FactoryRegistry<FactoryType>::find_with_tool(const char* name) const -> Vector<FactoryEntry> {
        Vector<FactoryEntry> result;
        for (const auto& entry : _entries) {
            if (std::strcmp(entry.tool_name, name) == 0) {
                result.push_back(entry);
            }
        }
        return result;
    }

    template<typename FactoryType>
    auto PluginRegistry::Impl::FactoryRegistry<FactoryType>::find(const char* plugin_name, const char* tool_name) const -> FactoryEntry {
        if (_entries.empty()) return FactoryEntry{};

        FactoryEntry target { plugin_name, tool_name, nullptr, nullptr };
        auto res = std::lower_bound(_entries.begin(), _entries.end(), target);
        
        if (res != _entries.end() && 
            std::strcmp(res->plugin_name, plugin_name) == 0 && 
            std::strcmp(res->tool_name, tool_name) == 0) 
        {
            return *res;
        }
        return FactoryEntry{};
    }

    template<typename FactoryType>
    auto PluginRegistry::Impl::FactoryRegistry<FactoryType>::findSmart(const char* name) const -> FactoryEntry {
        std::string query(name);
        size_t dot_pos = query.find('.');

        //  exact match (.)
        if (dot_pos != std::string::npos) {
            std::string plugin = query.substr(0, dot_pos);
            std::string tool = query.substr(dot_pos + 1);
            return find(plugin.c_str(), tool.c_str());
        }

        //  smart tool search
        auto candidates = find_with_tool(name);
        if (candidates.empty()) return FactoryEntry{};
        if (candidates.size() > 1) {
            throw std::runtime_error("Ambiguous tool name '" + query + "'. Please specify plugin prefix (e.g. 'plugin.tool').");
        }
        return candidates.front();
    }

    template<typename FactoryType>
    bool PluginRegistry::Impl::FactoryRegistry<FactoryType>::has(const char* plugin_name, const char* tool_name) const {
        return find(plugin_name, tool_name).factory != nullptr;
    }

    template<typename FactoryType>
    bool PluginRegistry::Impl::FactoryRegistry<FactoryType>::hasSmart(const char* name) const {
        try { return findSmart(name).factory != nullptr; }
        catch(...) { return false; }
    }

    template<typename FactoryType>
    auto PluginRegistry::Impl::FactoryRegistry<FactoryType>::push(const char* plugin, const char* tool, const char* desc, FactoryType factory) -> FactoryRegistry<FactoryType>& {
        FactoryEntry entry {plugin, tool, desc, factory};
        
        if (_entries.empty()) {
            _entries.push_back(std::move(entry));
            return *this;
        }

        auto insertion_point = std::lower_bound(_entries.begin(), _entries.end(), entry);

        if (insertion_point != _entries.end() && 
            std::strcmp(insertion_point->plugin_name, plugin) == 0 &&
            std::strcmp(insertion_point->tool_name, tool) == 0) 
        {
            throw std::runtime_error(std::string("Factory `") + plugin + "." + tool + "` already exists!");
        }

        _entries.insert(insertion_point, std::move(entry));
        return *this;
    }

    template<typename FactoryType>
    Vector<ToolInfo> PluginRegistry::Impl::FactoryRegistry<FactoryType>::getAvailableTools() const {
        Vector<ToolInfo> info;
        for (const auto& entry : _entries) {
            info.push_back({
                entry.plugin_name ? entry.plugin_name : "", 
                entry.tool_name ? entry.tool_name : "", 
                entry.description ? entry.description : ""
            });
        }
        return info;
    }


    //  ------------------------
    //      PLUGIN REGISTRY
    //  ------------------------

    PluginRegistry::PluginRegistry() : _impl(new Impl()) {}
    PluginRegistry::~PluginRegistry() = default;

    PluginRegistry::PluginRegistry(PluginRegistry&&) noexcept = default;
    PluginRegistry& PluginRegistry::operator=(PluginRegistry&&) noexcept = default;

    //  Dynamic Library

    void PluginRegistry::loadFromFile(const char* path) {
        DynamicLibrary lib(path);

        IPlugin::Ptr plugin = lib.getSymbol();

        loadPlugin(std::move(plugin));
        this->_impl->_libraries.push_back(std::move(lib));
    }

    //  Lifecycle

    void PluginRegistry::loadPlugin(IPlugin::Ptr plugin) {
        if (!plugin) return;

        const char* p_name = plugin->getName();
        
        _impl->_plugin_infos.push_back({
            p_name, 
            plugin->getVersion(), 
            plugin->getDescription()
        });

        PluginManifest m = plugin->getManifest();

        for (auto& lex : m.lexers) _impl->_lexers.push(p_name, lex.name, lex.description, lex.factory);
        for (auto& par : m.parsers) _impl->_parsers.push(p_name, par.name, par.description, par.factory);
        for (auto& vis : m.visitors) _impl->_visitors.push(p_name, vis.name, vis.description, vis.factory);

        _impl->_manifests[p_name] = std::move(m);

        _impl->_plugins.push_back(std::move(plugin));
    }

    void PluginRegistry::fillModule(Module& mod, const char* plugin_name) const {
        auto it = _impl->_manifests.find(plugin_name);
        if (it != _impl->_manifests.end()) {
            for (const auto& var : it->second.variables) {
                mod.getTable().defineVariable(var.name, var.value, var.is_const);
            }
            for (const auto& func : it->second.functions) {
                mod.getTable().defineFunction(func.name, ICallable::Ptr(func.function()));
            }
        } else {
            throw std::runtime_error(std::string("Cannot fill module: plugin '") + plugin_name + "' is not loaded.");
        }
    }

    //  DISCOVERY API
    Vector<PluginInfo> PluginRegistry::getLoadedPlugins() const { return _impl->_plugin_infos; }
    Vector<ToolInfo> PluginRegistry::getAvailableLexers() const { return _impl->_lexers.getAvailableTools(); }
    Vector<ToolInfo> PluginRegistry::getAvailableParsers() const { return _impl->_parsers.getAvailableTools(); }
    Vector<ToolInfo> PluginRegistry::getAvailableVisitors() const { return _impl->_visitors.getAvailableTools(); }

    const PluginManifest& PluginRegistry::getPluginManifest(const char* plugin_name) const {
        auto it = _impl->_manifests.find(plugin_name);
        if (it != _impl->_manifests.end()) {
            return it->second;
        }
        throw std::runtime_error(std::string("Plugin '") + plugin_name + "' manifest not found!");
    }

    //  FACTORY API (Exact)
    ILexer::Ptr PluginRegistry::createLexer(const char* p, const char* n) const {
        auto f = _impl->_lexers.find(p, n).factory;
        if (!f) throw std::runtime_error(std::string("Lexer not found: ") + p + "." + n);
        return ILexer::Ptr(f());
    }
    IParser::Ptr PluginRegistry::createParser(const char* p, const char* n) const {
        auto f = _impl->_parsers.find(p, n).factory;
        if (!f) throw std::runtime_error(std::string("Parser not found: ") + p + "." + n);
        return IParser::Ptr(f());
    }
    IVisitor::Ptr PluginRegistry::createVisitor(const char* p, const char* n, ExecutionContext* ctx) const {
        auto f = _impl->_visitors.find(p, n).factory;
        if (!f) throw std::runtime_error(std::string("Contextual Visitor not found: ") + p + "." + n);
        return IVisitor::Ptr(f(ctx));
    }

    //  FACTORY API (Smart)
    ILexer::Ptr PluginRegistry::createLexer(const char* n) const {
        auto f = _impl->_lexers.findSmart(n).factory;
        if (!f) throw std::runtime_error(std::string("Lexer not found: ") + n);
        return ILexer::Ptr(f());
    }
    IParser::Ptr PluginRegistry::createParser(const char* n) const {
        auto f = _impl->_parsers.findSmart(n).factory;
        if (!f) throw std::runtime_error(std::string("Parser not found: ") + n);
        return IParser::Ptr(f());
    }
    IVisitor::Ptr PluginRegistry::createVisitor(const char* n, ExecutionContext* ctx) const {
        auto f = _impl->_visitors.findSmart(n).factory;
        if (!f) throw std::runtime_error(std::string("Contextual Visitor not found: ") + n);
        return IVisitor::Ptr(f(ctx));
    }

    //  VALIDATION API
    bool PluginRegistry::hasLexer(const char* p, const char* n) const { return _impl->_lexers.has(p, n); }
    bool PluginRegistry::hasParser(const char* p, const char* n) const { return _impl->_parsers.has(p, n); }
    bool PluginRegistry::hasVisitor(const char* p, const char* n) const { return _impl->_visitors.has(p, n); }

    bool PluginRegistry::hasLexer(const char* n) const { return _impl->_lexers.hasSmart(n); }
    bool PluginRegistry::hasParser(const char* n) const { return _impl->_parsers.hasSmart(n); }
    bool PluginRegistry::hasVisitor(const char* n) const { return _impl->_visitors.hasSmart(n); }

} // namespace Synapse
