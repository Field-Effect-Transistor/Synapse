//  /core/include/synapse/Value.hpp
#pragma once

#include "interface/ICustomValue.hpp"

#include <cstdint>
#include <string>

namespace Synapse {

    class Value {
    public:

        enum class Type : uint8_t {
            NUMBER,
            BOOLEAN,
            CUSTOM_OBJECT
        };

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
        
        ~Value() {
            if(isCustom()) {
                static_cast<ICustomValue*>(_data.as_custom)->destroy();
            }
        }

        Value(Value&& other);
        Value(const Value& other);
        Value& operator=(Value&& other);
        Value& operator=(const Value& other);

        inline Type type() const { return _type; }

        inline bool isNumber() const { return _type == Type::NUMBER; }
        inline bool isBoolean() const {return _type == Type::BOOLEAN; }
        inline bool isCustom() const { return _type == Type::CUSTOM_OBJECT; }

        double getNumber() const;
        bool getBool() const;
        ICustomValue* getCustom() const;

        std::string to_str() const;

    };  //  class   Value

}   //  namespace   Synapse
