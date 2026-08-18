//  /plugin/PrinterLib/src/StringValue.cpp
#include "StringValue.hpp"

#include <utility>

namespace PrinterLib {

    StringValue::StringValue() = default;

    StringValue::StringValue(const StringValue& string_value) : value(string_value.value) {}

    StringValue::StringValue(std::string string) : value(std::move(string)) {}

    StringValue::~StringValue() = default;

    void StringValue::destroy() { delete this; }

    Synapse::ICustomValue* StringValue::clone() {
        return new StringValue(*this);
    }

    const char* StringValue::to_str() const {
        return value.c_str();
    }

}   //  namespace   PrinterLib
