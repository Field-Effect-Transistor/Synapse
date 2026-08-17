// /core/include/synapse/CalculatorBuilder.hpp
#pragma once

#include "synapse/Calculator.hpp"

namespace Synapse {

    class PluginRegistry;

    class CalculatorBuilder final {
        Calculator::Recipe  _recipe;
        PluginRegistry*     _registry;

    public:
        explicit CalculatorBuilder(PluginRegistry* reg) : _registry(reg) {}
        ~CalculatorBuilder() = default;

        CalculatorBuilder(const CalculatorBuilder&) = delete;
        CalculatorBuilder& operator=(const CalculatorBuilder&) = delete;

        CalculatorBuilder(CalculatorBuilder&&) = default;
        CalculatorBuilder& operator=(CalculatorBuilder&&) = default;

        CalculatorBuilder& useLexer(const char* name);
        CalculatorBuilder& useParser(const char* name);
        CalculatorBuilder& useEvaluator(const char* name);
        CalculatorBuilder& addOptimizer(const char* name);
        CalculatorBuilder& clearOptimizerList();

        CalculatorBuilder& applyRecipe(const Calculator::Recipe& recipe);

        Calculator build();

    };  //  class   CalculatorBuilder

}   //  namespace   Synapse
