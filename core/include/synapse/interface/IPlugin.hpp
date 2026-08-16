//  /core/include/synapse/interface/IPlugin.hpp
#pragma once
#include "IABIObject.hpp"

#include "synapse/Context.hpp"
#include "synapse/Value.hpp"

#include "UniquePtr.hpp"

namespace Synapse {
    struct ICallable;
    struct ILexer;
    struct IParser;
    struct IVisitor;

    struct PluginManifest {
        using CallableFactory           = ICallable* (*)();
        using LexerFactory              = ILexer* (*)();
        using ParserFactory             = IParser* (*)();
        using SimpleVisitorFactory      = IVisitor* (*)();
        using ContextualVisitorFactory  = IVisitor* (*)(Context*);

        struct VariableDecl {
            const char* name;
            const char* description;
            Value       value;
            bool        is_const = true;
        };  //  struct  VariableDecl

        struct FunctionDecl {
            const char*     name;
            const char*     description;
            CallableFactory function;
        };  //  struct  FunctionDecl

        struct LexerDecl {
            const char*     name;
            const char*     description;
            LexerFactory    factory;
        };  //  struct  LexerDecl

        struct ParserDecl {
            const char*     name;
            const char*     description;
            ParserFactory   factory;
        };  //  struct  ParserDecl

        struct SimpleVisitorDecl {
            const char*             name;
            const char*             description;
            SimpleVisitorFactory    factory;
        };  //  struct  SimpleVisitorDecl

        struct ContextualVisitorDecl {
            const char*                 name;
            const char*                 description;
            ContextualVisitorFactory    factory;
        };  //  struct  ContextualVisitorDecl

        Vector<VariableDecl>    variables;
        Vector<FunctionDecl>    functions;

        Vector<LexerDecl>               lexers;
        Vector<ParserDecl>              parsers;
        Vector<SimpleVisitorDecl>       simple_visitors;
        Vector<ContextualVisitorDecl>   contextual_visitors;

    };  //  PluginManifest

    struct IPlugin : IABIObject<IPlugin> {
        virtual ~IPlugin() = default;
        virtual void destroy() = 0;

        virtual const char* getName() const = 0;
        virtual const char* getVersion() const = 0;
        virtual const char* getDescription() const = 0;

        virtual PluginManifest getManifest() const = 0;
    };  //  struct  IPlugin

    struct PluginDeleter {
        void operator()(IPlugin* ptr) const { if (ptr) ptr->destroy(); }
    };

}   //  namespace   Synapse
