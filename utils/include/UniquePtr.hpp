//  /utils/include/UniquePtr.hpp

#include <stddef.h>

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
        UniquePtr(UniquePtr&& ptr) {
            _ptr = ptr._ptr;
            ptr._ptr = nullptr;
        }    

        UniquePtr& operator=(const UniquePtr&) = delete;
        UniquePtr& operator=(UniquePtr&& ptr) {
            if (&ptr != this) {
                DELETER{}(_ptr);
                _ptr = ptr._ptr;
                ptr._ptr = nullptr;
            }
            return *this;
        }

        UniquePtr() {
            _ptr = nullptr;
        }

        explicit UniquePtr(T* ptr) {
            _ptr = ptr;
        }

        explicit UniquePtr(nullptr_t ptr) {
            _ptr = nullptr;
        }

        ~UniquePtr() {
            DELETER{}(_ptr);
        }

        T& operator*() {
            return *_ptr;
        }

        T* operator->() {
            return _ptr;
        }

        explicit operator bool() const {
            return _ptr != nullptr;
        }

        T* get() {
            return _ptr;
        }

        T* release() {
            T* ptr = _ptr;
            _ptr = nullptr;
            return ptr;
        }

        void reset(T* new_ptr) {
            DELETER{}(_ptr);
            _ptr = new_ptr;
        }

    };  //  class   UniquePtr

}   //  namespace   Hermes
