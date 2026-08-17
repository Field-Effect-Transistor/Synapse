//  /core/include/synapse/Calculator.hpp
#pragma once

#include <string>

#include "Vector.hpp"
#include "Value.hpp"

namespace Synapse {

    //  FWD DECL
    class   PluginRegistry;
    class   Context;

    class Calculator {
    public:
        struct Recipe {
            std::string lexer;
            std::string parser;
            std::string evaluator;

            Vector<std::string> optimizers;
        };  //  struct  Recipe

    private:
        PluginRegistry* _registry = nullptr;
        Recipe          _recipe;

    public:
        Calculator() = default;
        Calculator(
            PluginRegistry* reg,
            Recipe          rec
        ) : _registry(reg), _recipe(rec) {};

        ~Calculator() = default;

        Calculator(const Calculator&) = delete;
        Calculator& operator=(const Calculator&) = delete;

        Calculator(Calculator&& calc) = default;
        Calculator& operator=(Calculator&& calc) = default;

        Value evaluate(const std::string& code, Context* execution_context);

    };  //  class   Calculator

}   //  namespace   Synapse
