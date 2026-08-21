//  /core/src/Calculator.cpp
#include "synapse/Calculator.hpp"

#include "synapse/PluginRegistry.hpp"
#include "synapse/ExecutionContext.hpp"
#include "synapse/StreamReader.hpp"

#include "UniquePtr.hpp"

#include <sstream>

namespace Synapse {

    Calculator::Calculator(PluginRegistry* reg, Recipe rec) 
        : _registry(reg), _recipe(std::move(rec)) 
    {
        if (!_registry) throw std::runtime_error("Calculator: PluginRegistry is missing!");
        if (_recipe.lexer.empty()) throw std::runtime_error("Calculator: Lexer is missing in recipe!");
        if (_recipe.parser.empty()) throw std::runtime_error("Calculator: Parser is missing in recipe!");
        if (_recipe.evaluator.empty()) throw std::runtime_error("Calculator: Evaluator is missing in recipe!");
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

        for (const auto& opt_name : _recipe.optimizers) {
            auto optimizer = _registry->createSimpleVisitor(opt_name.c_str());
            ast->accept(*optimizer);
        }

        auto evaluator = _registry->createContextualVisitor(_recipe.evaluator.c_str(), execution_context);
        Value result = ast->accept(*evaluator);

        return result;
    }

}   //  namespace   Synapse
