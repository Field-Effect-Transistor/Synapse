#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

#include "internal/Lexer.hpp"
#include "internal/Parser.hpp"
#include "internal/MathEvaluator.hpp"
#include "synapse/ContextManager.hpp"
#include "synapse/Context.hpp"
#include "synapse/Callable.hpp"
#include "synapse/Exceptions.hpp"
#include "synapse/Value.hpp"
#include "synapse/interface/IReader.hpp"

using namespace Synapse;
using namespace Synapse::Internal;

class StreamReader : public IReader {
    std::istream& _is;
public:
    StreamReader(std::istream& is) : _is(is) {}

    size_t read(char* buffer, size_t size) override {
        _is.read(buffer, static_cast<std::streamsize>(size));
        return static_cast<size_t>(_is.gcount());
    }

    bool isEOF() const override {
        return _is.eof();
    }
};

void setupStandardLibrary(Context* ctx) {
    ctx->defineVariable("pi", Value(3.141592653589793), true);
    ctx->defineVariable("e",  Value(2.718281828459045), true);

    // Функція max(a, b)
    ctx->defineFunction("max", make_callable(2, [](const Vector<Value>& args) {
        double a = args[0].getNumber();
        double b = args[1].getNumber();
        return Value(a > b ? a : b);
    }));

    // Функція min(a, b)
    ctx->defineFunction("min", make_callable(2, [](const Vector<Value>& args) {
        double a = args[0].getNumber();
        double b = args[1].getNumber();
        return Value(a < b ? a : b);
    }));

    // Функція sin(x)
    ctx->defineFunction("sin", make_callable(1, [](const Vector<Value>& args) {
        return Value(std::sin(args[0].getNumber()));
    }));
}

int main(int argc, char* argv[]) {
    // 1. Перевіряємо наявність аргументів
    if (argc < 2) {
        std::cerr << "Usage: synapse \"<expression>\"\n";
        std::cerr << "Example: synapse \"max(1, 2) * pi\"\n";
        return 1;
    }

    // Збираємо всі аргументи в один рядок (якщо користувач забув лапки)
    std::string expression = argv[1];
    for (int i = 2; i < argc; ++i) {
        expression += " ";
        expression += argv[i];
    }

    try {
        // 2. Лексичний аналіз (Lexer)
        std::istringstream stream(expression);
        StreamReader reader(stream);
        
        Lexer lexer;
        lexer.init(&reader);
        
        Vector<Token> tokens;
        while (true) {
            Token t = lexer.getNextToken();
            bool is_eof = (t.type == StandardToken::END_OF_FILE);
            
            // Пропускаємо коментарі
            if (t.type != StandardToken::COMMENT) {
                tokens.push_back(std::move(t));
            }
            
            if (is_eof) break;
        }

        // 3. Синтаксичний аналіз (Parser)
        Parser parser;
        ASTNodePtr ast = parser.parse(tokens);

        // 4. Налаштування контексту виконання
        ContextManager ctx_manager;
        Context* global_ctx = ctx_manager.getGlobalScope();
        setupStandardLibrary(global_ctx);

        // 5. Обчислення (MathEvaluator)
        MathEvaluator evaluator(global_ctx);
        Value result = ast->accept(evaluator);

        // 6. Вивід результату
        std::cout << result.to_str() << std::endl;

    } catch (const SyntaxError& e) {
        std::cerr << "❌ " << e.what() << std::endl;
        return 2;
    } catch (const RuntimeError& e) {
        std::cerr << "❌ " << e.what() << std::endl;
        return 3;
    } catch (const HermesException& e) {
        std::cerr << "❌ Synapse Error: " << e.what() << std::endl;
        return 4;
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal Error: " << e.what() << std::endl;
        return 5;
    }

    return 0;
}
