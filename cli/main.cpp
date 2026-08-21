// src/main.cpp
#include <iostream>
#include <string>

#include "synapse/SynapseEngine.hpp"
#include "synapse/Exceptions.hpp"
#include "synapse/Value.hpp"

using namespace Synapse;

// Оголошуємо C-функцію створення вбудованого плагіна стандартної бібліотеки
namespace Synapse::Plugin {
    extern "C" IPlugin* create_stdlib_plugin();
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
        // 2. Ініціалізуємо рушій Synapse
        SynapseEngine engine;

        // 3. Завантажуємо стандартну бібліотеку (вона сама додасть pi, e, sin, max, min, лексери, парсери тощо)
        engine.loadPlugin(IPlugin::Ptr(Plugin::create_stdlib_plugin()));

        // 4. Створюємо рецепт калькулятора (якими інструментами з плагінів користуватися)
        Calculator::Recipe recipe;
        recipe.lexer = "stdlib.Standard Lexer";
        recipe.parser = "stdlib.Standard Parser";
        recipe.evaluator = "stdlib.Math Evaluator";

        // 5. Створюємо головну сесію (середовище виконання) та перемикаємось на неї
        engine.createSession("main", recipe);

        // 6. Виконуємо код
        Value result = engine.evaluate(expression);

        // 7. Виводимо результат
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
