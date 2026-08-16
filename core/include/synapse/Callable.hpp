//  /core/include/synapse/Callable.hpp
#pragma once
#include "interface/ICallable.hpp"
#include <type_traits>

namespace Synapse {

    template<typename CallableType>
    class Functional : public ICallable {
        CallableType    _func;
        size_t          _arity;

    public:
        Functional(CallableType f, size_t arity) 
            : _func(std::move(f)), _arity(arity) {
                static_assert(std::is_invocable_v<CallableType, const Vector<Value>&>, 
                              "Callable must match signature: Value(const Vector<Value>&)");
            }

        ~Functional() override = default;

        void destroy() override {
            delete this;
        }

        Value operator()(const Vector<Value>& args) override {
            if (_arity != VARITY && _arity != args.size()) {
                throw ArityMismatchError(_arity, args.size());
            }
            return _func(args);
        }

        size_t arity() const override {
            return _arity;
        }

        explicit operator bool() const override {
            return true;
        }
    };  //  class Functional

    template<typename CallableType>
    ICallable::Ptr make_callable(size_t arity, CallableType&& func) {
        using CleanType = std::decay_t<CallableType>;
        ICallable* raw_ptr = new Functional<CleanType>(std::forward<CallableType>(func), arity);
        return ICallable::Ptr(raw_ptr);
    }

}   // namespace Synapse
