//  /core/include/synapse/interface/ICallable.hpp
#pragma once

#include "Value.hpp"
#include "Vector.hpp"
#include "UniquePtr.hpp"

#include "Exceptions.hpp"

namespace Synapse {
    struct ICallable {
        static constexpr size_t VARITY = static_cast<size_t>(-1);

        virtual ~ICallable() = default;
        virtual void destroy() = 0;

        virtual Value operator()(const Vector<Value>&) = 0;
        virtual explicit operator bool() const = 0;

        virtual size_t arity() const = 0;
    };  //  struct  ICallable

    struct CallableDeleter {
        void operator()(ICallable* ptr) {
            if (ptr) {
                ptr->destroy();
            }
        }
    };  //  struct  CallableDeleer

    using CallablePtr = UniquePtr<ICallable, CallableDeleter>;
}   //  namespace   Synapse
