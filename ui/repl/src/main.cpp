//  /ui/repl/src/main.cpp

#include "ReplApp.hpp"
#include <iostream>
#include <exception>

#ifdef _WIN32
    #include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    try {
        Synapse::Repl::ReplApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "\n\033[31m[FATAL ERROR]\033[0m " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n\033[31m[FATAL ERROR]\033[0m Unknown exception occurred.\n";
        return 2;
    }

    return 0;
}
