//  /core/include/synapse/Calculator.hpp
#pragma once

#include <string>
#include <functional>

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

        using RoleWarningCallback = std::function<void(const std::string& message)>;

    private:
        PluginRegistry*     _registry = nullptr;
        Recipe               _recipe;
        RoleWarningCallback  _on_role_warning;

    public:
        Calculator() = delete;

        Calculator(PluginRegistry* reg, Recipe rec, RoleWarningCallback on_role_warning = nullptr);

        ~Calculator() = default;

        Calculator(const Calculator&) = delete;
        Calculator& operator=(const Calculator&) = delete;

        Calculator(Calculator&& calc) = default;
        Calculator& operator=(Calculator&& calc) = default;

        Value evaluate(const std::string& code, ExecutionContext* execution_context);

    };  //  class   Calculator

}   //  namespace   Synapse
