//  /include/core/Value.hpp
#pragma once

#include <cstdint>
#include <stdexcept>

namespace Hermes {

    struct ICustomValue {
        virtual ~ICustomValue() = default;
        virtual void destroy() = 0;
    };  //  struct  ICustomValue

    class Value {
    public:

        enum class Type : uint8_t {
            NUMBER,
            BOOLEAN,
            CUSTOM_OBJECT
        };  //  

    private:
        Type    _type;

        union {
            double          as_number;
            bool            as_bool;
            ICustomValue*   as_custom;
        }       _data;

    public:
        Value() : _type(Type::NUMBER) { _data.as_number = .0; }
        Value(double n) : _type(Type::NUMBER) {_data.as_number = n; }
        Value(bool b) : _type(Type::BOOLEAN) { _data.as_bool = b; }
        Value(ICustomValue* c) : _type(Type::CUSTOM_OBJECT) { _data.as_custom = c; }

        inline Type type() const { return _type; }

        bool isNumber() const { return _type == Type::NUMBER; }
        bool isBoolean() const {return _type == Type::BOOLEAN; }
        bool isCustom() const { return _type == Type::CUSTOM_OBJECT; }

        double getNumber() const {
            if(isNumber()) {
                return _data.as_number;
            }
            throw std::runtime_error("Not a number!!!");
        }

        bool getBool() const {
            if(isBoolean()) {
                return _data.as_bool;
            }
            throw std::runtime_error("Not a boolean!!!");
        }

        ICustomValue* getCustom() const {
            if(isCustom()) {
                return _data.as_custom;
            }
            throw std::runtime_error("Not a custom object!!!");
        }

    };  //  class   Value

}   //  namespace   Hermes
