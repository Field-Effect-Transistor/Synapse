//  src/core/main.cpp
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include "Lexer.hpp"

void runTest(const std::string& testName, const std::string& code) {
    std::cout << "========== ТЕСТ: " << testName << " ==========\n";
    std::cout << "Код:\n" << code << "\n--------------------------------\n";

    std::istringstream stream(code);
    Hermes::Lexer lexer(stream, 15);

    while (true) {
        Hermes::Token t = lexer.fetchNextToken();
        
        t.print(std::cout); // Використовуємо твій новий метод!
        std::cout << '\n';

        if (t.type == Hermes::StandardToken::END_OF_FILE) {
            break;
        }
    }
    std::cout << "\n\n";
}

int main() {
    // 1. Базова математика та ігнорування пробілів
    runTest("Базова математика", 
            "12.5 + 45 * (2 - 0.5) / 3 ^ 2");

    // 2. Ідентифікатори, розділювачі та багаторядковість
    runTest("Змінні та функції", 
            "max(var_1, 100);\nvar_2 % 3;");

    // 3. Коментарі посеред коду (перевірка межі чанків)
    // Символ \r\n перевіряє сумісність з Windows
    runTest("Коментарі", 
            "100 / // це коментар, він має бути окремим токеном\r\n 2");

    // 4. Помилки користувача (Неправильні числа та невідомі символи)
    // Увага на '=' та '@' - їх немає в StandardToken, лексер має видати ERROR
    runTest("Синтаксичні помилки", 
            "12.34.56 \n 45abc \n x = 10 @");

    return 0;
}
