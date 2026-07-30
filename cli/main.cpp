//  /cli/main.cpp
#include <iostream>
#include <sstream>
#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "ASTPrinter.hpp"

using namespace Hermes;

void runParserTest(const std::string& testName, const std::string& code) {
    std::cout << "========== ТЕСТ: " << testName << " ==========\n";
    std::cout << "Вихідний код : " << code << "\n";

    try {
        // 1. Лексичний аналіз
        std::istringstream stream(code);
        Lexer lexer(stream, 1024);
        Vector<Token> tokens;
        
        while (true) {
            Token t = lexer.fetchNextToken();
            if (t.type != StandardToken::COMMENT) {
                tokens.push_back(std::move(t));
            }
            if (tokens.back().type == StandardToken::END_OF_FILE) break;
        }

        // 2. Синтаксичний аналіз
        Parser parser(tokens);
        ASTNodePtr root = parser.parse(tokens);

        if (!root) {
            std::cout << "ERROR: empty ast (EOF).\n\n";
            return;
        }

        // 3. Друкуємо AST
        ASTPrinter printer;
        root->accept(printer);
        std::cout << "AST (LISP)   : " << printer.result() << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    std::cout << "------------------------------------------------\n\n";
}

int main() {
    // 1. Перевірка пріоритетів (+ та *)
    runParserTest("Пріоритет операцій", "2 + 3 * 4");

    // 2. Перевірка дужок (зміна пріоритету)
    runParserTest("Вплив дужок", "(2 + 3) * 4");

    // 3. Унарні оператори та відсотки
    runParserTest("Унарний мінус і відсотки", "-50%");

    // 4. Правоасоціативність ступеня
    runParserTest("Ступінь (Правоасоціативність)", "2 ^ 3 ^ 2");

    // 5. Виклики функцій з аргументами
    runParserTest("Функції та Змінні", "max(10, 20) * x");

    // 6. Спеціальний тест на обробку синтаксичних помилок
    runParserTest("Відловлювання помилок", "5 * (10 + 2");

    return 0;
}
