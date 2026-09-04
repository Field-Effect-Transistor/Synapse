//  /core/src/Calculator.cpp
#include "synapse/Calculator.hpp"

#include "synapse/PluginRegistry.hpp"
#include "synapse/ExecutionContext.hpp"
#include "synapse/StreamReader.hpp"
#include "synapse/interface/IVisitor.hpp"

#include "UniquePtr.hpp"

#include <sstream>

namespace Synapse {

    Calculator::Calculator(PluginRegistry* reg, Recipe rec, RoleWarningCallback on_role_warning)
        : _registry(reg), _recipe(std::move(rec)), _on_role_warning(std::move(on_role_warning))
    {
        if (!_registry) throw std::runtime_error("Calculator: PluginRegistry is missing!");
        if (_recipe.lexer.empty()) throw std::runtime_error("Calculator: Lexer is missing in recipe!");
        if (_recipe.parser.empty()) throw std::runtime_error("Calculator: Parser is missing in recipe!");
        if (_recipe.producer.empty()) throw std::runtime_error("Calculator: Evaluator is missing in recipe!");
    }

    Value Calculator::evaluate(const std::string& code, ExecutionContext* execution_context) {
        if (!_registry) throw std::runtime_error("Calculator: PluginRegistry is missing!");
        if (!execution_context) throw std::runtime_error("Calculator: Execution ExecutionContext is missing!");

        std::istringstream code_stream(code);
        StreamReader reader(code_stream);

        auto lexer = _registry->createLexer(_recipe.lexer.c_str());
        lexer->init(&reader);

        Vector<Token> tokens;
        while (true) {
            Token t = lexer->getNextToken();
            bool is_eof = (t.type == StandardToken::END_OF_FILE);

            if (t.type != StandardToken::COMMENT) {
                tokens.push_back(std::move(t));
            }

            if (is_eof) break;
        }

        auto parser = _registry->createParser(_recipe.parser.c_str());
        IASTNode::Ptr ast = parser->parse(tokens);

        if (!ast) {
            return Value(0.0);
        }

        for (const auto& opt_name : _recipe.preprocessors) {
            auto optimizer = _registry->createVisitor(opt_name.c_str(), execution_context);

            if (_on_role_warning && optimizer->getRole() != IVisitor::Role::Preprocessor) {
                _on_role_warning(
                    "Optimizer step '" + opt_name +
                    "' has role Producer, but is used as a Preprocessor step. "
                    "This may silently affect the evaluation result.");
            }

            ast->accept(*optimizer);
        }

        auto producer = _registry->createVisitor(_recipe.producer.c_str(), execution_context);

        if (_on_role_warning && producer->getRole() != IVisitor::Role::Producer) {
            _on_role_warning(
                "Evaluator '" + _recipe.producer +
                "' has role Preprocessor, but is used as the final producer. "
                "The returned value may not be meaningful.");
        }

        Value result = ast->accept(*producer);

        return result;
    }

}   //  namespace   Synapse
