//  /plugin/PrinterLib/src/StringValue.hpp
#pragma once
#include "synapse/interface/ICustomValue.hpp"

#include <string>

namespace PrinterLib {

    class StringValue final : public Synapse::ICustomValue {
        std::string value;

    public:
        StringValue();
        StringValue(const StringValue& string_value);
        StringValue(std::string string);

        ~StringValue();

        void destroy() override;

        ICustomValue* clone() override;
        const char* to_str() const override;
    };  //  class   StringValue

}   //  namespace   PrinterLib
