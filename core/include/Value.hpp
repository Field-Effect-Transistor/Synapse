//  /include/core/Value.hpp
#pragma once

#include <cstdint>
#include "Exceptions.hpp"

namespace Hermes {

    struct ICustomValue {
        virtual ~ICustomValue() = default;
        
        virtual ICustomValue* clone() = 0;
        virtual void destroy() = 0;
        virtual const char* to_str() const = 0;
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
        
        ~Value() {
            if(isCustom()) {
                static_cast<ICustomValue*>(_data.as_custom)->destroy();
            }
        }

        Value(Value&& other) {
            _type = other._type;
            _data = other._data;
            
            other._type = Type::NUMBER; 
            other._data.as_number = 0;
        }

        Value& operator=(Value&& other) {
            if (this == &other) return *this;

            if (this->isCustom()) {
                static_cast<ICustomValue*>(this->_data.as_custom)->destroy();
            }

            _type = other._type;
            _data = other._data;
            
            other._type = Type::NUMBER; 
            other._data.as_number = 0;

            return *this;
        }

        Value(const Value& other) {
            _type = other._type;
            if (other.isCustom()) {
                _data.as_custom = other._data.as_custom->clone();
            } else {
                _data = other._data;
            }
        }

        Value& operator=(const Value& other) {
            if (this == &other) {
                return *this;
            }

            if (this->isCustom()) {
                static_cast<ICustomValue*>(this->_data.as_custom)->destroy();
            }

            _type = other._type;
            if (other.isCustom()) {
                _data.as_custom = other._data.as_custom->clone();
            } else {
                _data = other._data;
            }

            return *this;
        }

        inline Type type() const { return _type; }

        bool isNumber() const { return _type == Type::NUMBER; }
        bool isBoolean() const {return _type == Type::BOOLEAN; }
        bool isCustom() const { return _type == Type::CUSTOM_OBJECT; }

        double getNumber() const {
            if(isNumber()) {
                return _data.as_number;
            }
            throw TypeError("Expected a number, but got another type.");
        }

        bool getBool() const {
            if(isBoolean()) {
                return _data.as_bool;
            }
            throw TypeError("Expected a boolean, but got another type.");
        }

        ICustomValue* getCustom() const {
            if(isCustom()) {
                return _data.as_custom;
            }
            throw TypeError("Expected a custom object, but got another type.");
        }

        std::string to_str() const {
            switch(type()) {
                case Type::NUMBER: {
                    return std::to_string(_data.as_number);
                }
                case Type::BOOLEAN: {
                    return _data.as_bool ? "1" : "0";
                }
                case Type::CUSTOM_OBJECT: {
                    return _data.as_custom->to_str();
                }
                default:
                    return "UNKNOWN";
            }
        }

    };  //  class   Value

}   //  namespace   Hermes
