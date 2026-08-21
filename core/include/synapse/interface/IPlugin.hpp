//  /core/include/synapse/interface/IPlugin.hpp
#pragma once
#include "IABIObject.hpp"

#include "synapse/Value.hpp"
#include "synapse/interface/IVisitor.hpp"

#include "UniquePtr.hpp"
#include "Vector.hpp"

namespace Synapse {
    struct ICallable;
    struct ILexer;
    struct IParser;
    class  ExecutionContext;

    struct PluginManifest {
        using CallableFactory   = ICallable* (*)();
        using LexerFactory      = ILexer* (*)();
        using ParserFactory     = IParser* (*)();
        using VisitorFactory    = IVisitor* (*)(ExecutionContext*);

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

        struct VisitorDecl {
            const char*     name;
            const char*     description;
            IVisitor::Role  role;
            VisitorFactory  factory;
        };  //  struct  VisitorDecl


        Vector<VariableDecl>    variables;
        Vector<FunctionDecl>    functions;

        Vector<LexerDecl>       lexers;
        Vector<ParserDecl>      parsers;
        Vector<VisitorDecl>     visitors;

    };  //  PluginManifest

    struct IPlugin : IABIObject<IPlugin> {
        virtual ~IPlugin() = default;
        virtual void destroy() = 0;

        virtual const char* getName() const = 0;
        virtual const char* getVersion() const = 0;
        virtual const char* getDescription() const = 0;

        virtual PluginManifest getManifest() const = 0;
    };  //  struct  IPlugin

}   //  namespace   Synapse
