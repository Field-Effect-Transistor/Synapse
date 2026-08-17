//  /core/src/Calculator.cpp
#include "synapse/Calculator.hpp"

#include "synapse/PluginRegistry.hpp"
#include "synapse/Context.hpp"
#include "synapse/StreamReader.hpp"

#include "UniquePtr.hpp"

#include <sstream>

namespace Synapse {

    Value Calculator::evaluate(const std::string& code, Context* execution_context) {   
        if (!_registry) throw std::runtime_error("Calculator: PluginRegistry is missing!");
        if (!execution_context) throw std::runtime_error("Calculator: Execution Context is missing!");

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
