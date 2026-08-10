//  /core/src/Value.cpp
#include "Value.hpp"

#include "Exceptions.hpp"

namespace Synapse {

    Value::Value(Value&& other) {
        _type = other._type;
        _data = other._data;
        _is_const = other._is_const;
        
        other._type = Type::NUMBER; 
        other._data.as_number = 0;
        other._is_const = false;
    }

    Value& Value::operator=(Value&& other) {
        if (this == &other) return *this;

        if (this->isCustom()) {
            static_cast<ICustomValue*>(this->_data.as_custom)->destroy();
        }

        _type = other._type;
        _data = other._data;
        _is_const = other._is_const;
        
        other._type = Type::NUMBER; 
        other._data.as_number = 0;
        other._is_const = false;

        return *this;
    }

    Value::Value(const Value& other) {
        _type = other._type;
        _is_const = other._is_const;
        if (other.isCustom()) {
            _data.as_custom = other._data.as_custom->clone();
        } else {
            _data = other._data;
        }
    }

    Value& Value::operator=(const Value& other) {
        if (this == &other) {
            return *this;
        }

        if (this->isCustom()) {
            static_cast<ICustomValue*>(this->_data.as_custom)->destroy();
        }

        _type = other._type;
        _is_const = other._is_const;
        if (other.isCustom()) {
            _data.as_custom = other._data.as_custom->clone();
        } else {
            _data = other._data;
        }

        return *this;
    }

    double Value::getNumber() const {
        if(isNumber()) {
            return _data.as_number;
        }
        throw TypeError("Expected a number, but got another type.");
    }

    bool Value::getBool() const {
        if(isBoolean()) {
            return _data.as_bool;
        }
        throw TypeError("Expected a boolean, but got another type.");
    }

    ICustomValue* Value::getCustom() const {
        if(isCustom()) {
            return _data.as_custom;
        }
        throw TypeError("Expected a custom object, but got another type.");
    }

    std::string Value::to_str() const {
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

}   //  namespace   Synapse
