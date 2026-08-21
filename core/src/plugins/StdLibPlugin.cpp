//  /core/src/plugins/StdLibPlugin.cpp
#include "synapse/interface/IPlugin.hpp"
#include "synapse/Callable.hpp"
#include "synapse/ExecutionContext.hpp"

#include "internal/Lexer.hpp"
#include "internal/Parser.hpp"
#include "internal/MathEvaluator.hpp"

#include <cmath>

namespace Synapse::Plugin {

    Value math_sin(const Vector<Value>& args) { return Value(std::sin(args[0].getNumber())); }
    Value math_cos(const Vector<Value>& args) { return Value(std::cos(args[0].getNumber())); }
    Value math_tan(const Vector<Value>& args) { return Value(std::tan(args[0].getNumber())); }

    Value math_asin(const Vector<Value>& args) { return Value(std::asin(args[0].getNumber())); }
    Value math_acos(const Vector<Value>& args) { return Value(std::acos(args[0].getNumber())); }
    Value math_atan(const Vector<Value>& args) { return Value(std::atan(args[0].getNumber())); }

    Value math_sqrt(const Vector<Value>& args) { return Value(std::sqrt(args[0].getNumber())); }
    Value math_ln(const Vector<Value>& args) { return Value(std::log(args[0].getNumber())); }
    Value math_log10(const Vector<Value>& args) { return Value(std::log10(args[0].getNumber())); }
    Value math_exp(const Vector<Value>& args) { return Value(std::exp(args[0].getNumber())); }

    Value math_abs(const Vector<Value>& args) { return Value(std::abs(args[0].getNumber())); }
    Value math_round(const Vector<Value>& args) { return Value(std::round(args[0].getNumber())); }
    Value math_floor(const Vector<Value>& args) { return Value(std::floor(args[0].getNumber())); }
    Value math_ceil(const Vector<Value>& args) { return Value(std::ceil(args[0].getNumber())); }

    Value math_hypot(const Vector<Value>& args) { 
        return Value(std::hypot(args[0].getNumber(), args[1].getNumber())); 
    }

    Value math_max(const Vector<Value>& args) { 
        if (args.empty()) throw std::runtime_error("max() requires at least 1 argument");

        Value res = args[0];

        for(size_t i = 1; i < args.size(); ++i) {
            if (res.getNumber() < args[i].getNumber()) {
                res = args[i];
            }
        }
        return res;
    }

    Value math_min(const Vector<Value>& args) { 
        if (args.empty()) throw std::runtime_error("min() requires at least 1 argument");

        Value res = args[0];
        
        for(size_t i = 1; i < args.size(); ++i) {
            if (res.getNumber() > args[i].getNumber()) {
                res = args[i];
            }
        }
        return res;
    }


    ICallable* create_sin() { return new Functional<decltype(&math_sin)>(&math_sin, 1); }
    ICallable* create_cos() { return new Functional<decltype(&math_cos)>(&math_cos, 1); }
    ICallable* create_tan() { return new Functional<decltype(&math_tan)>(&math_tan, 1); }

    ICallable* create_asin() { return new Functional<decltype(&math_asin)>(&math_asin, 1); }
    ICallable* create_acos() { return new Functional<decltype(&math_acos)>(&math_acos, 1); }
    ICallable* create_atan() { return new Functional<decltype(&math_atan)>(&math_atan, 1); }

    ICallable* create_sqrt() { return new Functional<decltype(&math_sqrt)>(&math_sqrt, 1); }
    ICallable* create_ln() { return new Functional<decltype(&math_ln)>(&math_ln, 1); }
    ICallable* create_log10() { return new Functional<decltype(&math_log10)>(&math_log10, 1); }
    ICallable* create_exp() { return new Functional<decltype(&math_exp)>(&math_exp, 1); }

    ICallable* create_abs() { return new Functional<decltype(&math_abs)>(&math_abs, 1); }
    ICallable* create_round() { return new Functional<decltype(&math_round)>(&math_round, 1); }
    ICallable* create_floor() { return new Functional<decltype(&math_floor)>(&math_floor, 1); }
    ICallable* create_ceil() { return new Functional<decltype(&math_ceil)>(&math_ceil, 1); }

    ICallable* create_max() { return new Functional<decltype(&math_max)>(&math_max, ICallable::VARITY); }
    ICallable* create_min() { return new Functional<decltype(&math_min)>(&math_min, ICallable::VARITY); }
    ICallable* create_hypot() { return new Functional<decltype(&math_hypot)>(&math_hypot, 2); }


    ILexer*     create_lexer() { return new Internal::Lexer(); }
    IParser*    create_parser() { return new Internal::Parser(); }
    IVisitor*   create_visitor(ExecutionContext* ctx) { return new Internal::MathEvaluator(ctx); }


    class StdLibPlugin final : public IPlugin {
    public:
        ~StdLibPlugin() override = default;        
        void destroy() override { delete this; }

        const char* getName() const override { return "stdlib"; }
        const char* getVersion() const override { return "mvp 1.0.0"; }
        const char* getDescription() const override { return "Synapse Standard Library"; }

        PluginManifest getManifest() const override {
            PluginManifest m;

            m.variables.push_back({"pi",  "Pi constant (3.1415...)", Value(3.141592653589793), true});
            m.variables.push_back({"e",   "Euler's number (2.7182...)", Value(2.718281828459045), true});
            m.variables.push_back({"tau", "Tau constant (2*Pi)", Value(6.283185307179586), true});

            m.functions.push_back({"sin", "Sine function (radians)", &create_sin});
            m.functions.push_back({"cos", "Cosine function (radians)", &create_cos});
            m.functions.push_back({"tan", "Tangent function (radians)", &create_tan});

            m.functions.push_back({"asin", "Inverse sine (radians)", &create_asin});
            m.functions.push_back({"acos", "Inverse cosine (radians)", &create_acos});
            m.functions.push_back({"atan", "Inverse tangent (radians)", &create_atan});

            m.functions.push_back({"sqrt", "Square root", &create_sqrt});
            m.functions.push_back({"ln", "Natural logarithm (base e)", &create_ln});
            m.functions.push_back({"log10", "Common logarithm (base 10)", &create_log10});
            m.functions.push_back({"exp", "Exponential function (e^x)", &create_exp});

            m.functions.push_back({"abs", "Absolute value", &create_abs});
            m.functions.push_back({"round", "Round to nearest integer", &create_round});
            m.functions.push_back({"floor", "Round downwards", &create_floor});
            m.functions.push_back({"ceil", "Round upwards", &create_ceil});

            m.functions.push_back({"max", "Maximum of two values", &create_max});
            m.functions.push_back({"min", "Minimum of two values", &create_min});
            m.functions.push_back({"hypot", "Hypotenuse (sqrt(x^2 + y^2))", &create_hypot});

            m.lexers.push_back({"Standard Lexer", "", &create_lexer});
            m.parsers.push_back({"Standard Parser", "", &create_parser});
            m.contextual_visitors.push_back({"Math Evaluator", "Evaluate standard trees", &create_visitor});

            return m;
        }
    };  //  class StdLibPlugin

    extern "C" IPlugin* create_stdlib_plugin() {
        return new StdLibPlugin();
    }

}   //  namespace Synapse::Plugin
