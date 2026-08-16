//  /core/include/synapse/interface/ICallable.hpp
#pragma once
#include "IABIObject.hpp"

#include "synapse/Value.hpp"
#include "Vector.hpp"

#include "synapse/Exceptions.hpp"

namespace Synapse {
    struct ICallable : IABIObject<ICallable> {
        static constexpr size_t VARITY = static_cast<size_t>(-1);

        virtual ~ICallable() = default;

        virtual Value operator()(const Vector<Value>&) = 0;
        virtual explicit operator bool() const = 0;

        virtual size_t arity() const = 0;
    };  //  struct  ICallable

}   //  namespace   Synapse
