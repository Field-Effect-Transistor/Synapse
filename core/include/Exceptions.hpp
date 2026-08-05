//  /core/include/Exceptions.hpp
#pragma once
#include <stdexcept>
#include <string>

namespace Hermes {

    class HermesException : public std::runtime_error {
    public:
        explicit HermesException(const std::string& msg) : std::runtime_error(msg) {}
    };

    class SyntaxError : public HermesException {
    public:
        SyntaxError(const std::string& msg, int row, int col) 
            : HermesException("Syntax Error at [Row " + std::to_string(row) + 
                              ", Col " + std::to_string(col) + "]: " + msg) {}
        
        explicit SyntaxError(const std::string& msg) 
            : HermesException("Syntax Error: " + msg) {}
    };

    class RuntimeError : public HermesException {
    public:
        explicit RuntimeError(const std::string& msg) : HermesException("Runtime Error: " + msg) {}
    };

    class FunctionCallError : public RuntimeError {
    public:
        FunctionCallError(const std::string& msg) : RuntimeError(msg) {}
        FunctionCallError() : RuntimeError("Attempted to call an uninitialized function") {}
    };

    class ArityMismatchError : public FunctionCallError {
    public:
        ArityMismatchError(size_t expected, size_t got)
            : FunctionCallError( "Attempted to call an function with mismatch parametrs. "
                "Expected " + std::to_string(expected) + 
                " arguments, but got " + std::to_string(got) + ".") {}
    };

    class OutOfRangeError : public HermesException {
    public:
        explicit OutOfRangeError(const std::string& msg) 
            : HermesException("Out of Range Error: " + msg) {}
    };

    class TypeError : public RuntimeError {
    public:
        explicit TypeError(const std::string& msg) 
            : RuntimeError("Type Error: " + msg) {}
    };
}
