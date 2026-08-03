//  /utils/include/UniquePtr.hpp
#pragma once

#include <cstddef>

namespace Hermes {

    template<typename T>
    struct StandardDeleter {
        void operator()(T* ptr) const {
            delete ptr;
        }
    };  //  struct  StandartDelter

    template<
        typename T,
        typename DELETER = StandardDeleter<T>
    >
    class UniquePtr {
        T*  _ptr;
    public:
        UniquePtr(const UniquePtr&) = delete;    
        UniquePtr(UniquePtr&& ptr) noexcept {
            _ptr = ptr._ptr;
            ptr._ptr = nullptr;
        }    

        UniquePtr& operator=(const UniquePtr&) = delete;
        UniquePtr& operator=(UniquePtr&& ptr) noexcept {
            if (&ptr != this) {
                DELETER{}(_ptr);
                _ptr = ptr._ptr;
                ptr._ptr = nullptr;
            }
            return *this;
        }
        UniquePtr& operator=(std::nullptr_t) noexcept {
            reset();
            return *this;
        }

        explicit UniquePtr(T* ptr) noexcept { _ptr = ptr; }
        UniquePtr() noexcept { _ptr = nullptr; }
        UniquePtr(std::nullptr_t ptr) noexcept { _ptr = nullptr; }

        ~UniquePtr() noexcept { DELETER{}(_ptr); }

        T& operator*() noexcept { return *_ptr; }
        T& operator*() const noexcept { return *_ptr; }

        T* operator->() noexcept { return _ptr; }
        T* operator->() const noexcept { return _ptr; }

        explicit operator bool() const noexcept { return _ptr != nullptr; }
        bool operator==(const UniquePtr& other) const noexcept {
            return _ptr == other._ptr;
        }
        bool operator!=(const UniquePtr& other) const noexcept {
            return _ptr != other._ptr;
        }

        T* get() noexcept { return _ptr; }
        T* get() const noexcept { return _ptr; }

        T* release() noexcept {
            T* ptr = _ptr;
            _ptr = nullptr;
            return ptr;
        }

        inline void reset(T* new_ptr = nullptr) noexcept {
            if (new_ptr != _ptr) {
                DELETER{}(_ptr);
                _ptr = new_ptr;
            }
        }

    };  //  class   UniquePtr

}   //  namespace   Hermes
