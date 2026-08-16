//  /core/include/synapse/interface/IABIObject.hpp
#pragma once
#include "UniquePtr.hpp"

namespace Synapse {

    template<typename T>
    struct ABIDeleter {
        void operator()(T* ptr) const { if (ptr) ptr->destroy(); }
    };  //  struct  ABIDeleter

    template<typename T>
    struct IABIObject {
        virtual ~IABIObject() = default;
        virtual void destroy() = 0;
        
        using Ptr = UniquePtr<T, ABIDeleter<T>>;
    };  //  struct  IABIObject

}   //  namespace   Synapse
