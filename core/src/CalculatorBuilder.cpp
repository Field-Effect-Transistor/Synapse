//  /core/src/CalculatorBuilder.cpp
#include "synapse/CalculatorBuilder.hpp"

namespace Synapse {

    CalculatorBuilder& CalculatorBuilder::useLexer(const char* name) {
        _recipe.lexer = name;
        return *this;
    }

    CalculatorBuilder& CalculatorBuilder::useParser(const char* name) {
        _recipe.parser = name;
        return *this;
    }

    CalculatorBuilder& CalculatorBuilder::useEvaluator(const char* name) {
        _recipe.evaluator = name;
        return *this;
    }
    
    CalculatorBuilder& CalculatorBuilder::addOptimizer(const char* name) {
        _recipe.optimizers.push_back(name);
        return *this;
    }

    CalculatorBuilder& CalculatorBuilder::clearOptimizerList() {
        _recipe.optimizers.clear();
        return *this;
    }

    CalculatorBuilder& CalculatorBuilder::applyRecipe(const Calculator::Recipe& recipe) {
        _recipe = recipe;
        return *this;
    }

    Calculator CalculatorBuilder::build() {
        if (!_registry) {
            throw std::runtime_error("CalculatorBuilder: PluginRegistry is missing!");
        }

        if (_recipe.lexer.empty()) {
            throw std::runtime_error("CalculatorBuilder: Lexer is not specified in the recipe!");
        }
        if (_recipe.parser.empty()) {
            throw std::runtime_error("CalculatorBuilder: Parser is not specified in the recipe!");
        }
        if (_recipe.evaluator.empty()) {
            throw std::runtime_error("CalculatorBuilder: Evaluator is not specified in the recipe!");
        }

        return Calculator(_registry, std::move(_recipe));
    }

}   //  namespace   Synapse
