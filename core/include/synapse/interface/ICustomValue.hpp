//  /core/include/synapse/interface/ICustomValue.hpp
#pragma once

namespace Synapse {

    struct ICustomValue {
        virtual ~ICustomValue() = default;
        virtual void destroy() = 0;
        
        virtual ICustomValue* clone() = 0;
        virtual const char* to_str() const = 0;
    };  //  struct  ICustomValue
    
}   //  namespace   Synapse
