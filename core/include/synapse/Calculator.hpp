//  /core/include/synapse/Calculator.hpp
#pragma once

#include <string>

#include "Vector.hpp"
#include "Value.hpp"

namespace Synapse {

    //  FWD DECL
    class PluginRegistry;
    class ExecutionContext;

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
        Calculator() = delete;

        Calculator(PluginRegistry* reg, Recipe rec);

        ~Calculator() = default;

        Calculator(const Calculator&) = delete;
        Calculator& operator=(const Calculator&) = delete;

        Calculator(Calculator&& calc) = default;
        Calculator& operator=(Calculator&& calc) = default;

        Value evaluate(const std::string& code, ExecutionContext* execution_context);

    };  //  class   Calculator

}   //  namespace   Synapse
