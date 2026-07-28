//  /utils/include/ArenaAllocator.hpp
#pragma once

#include <cstdint>
#include <cstddef>
#include <new>

namespace Hermes {
    class ArenaAllocator {
        uint8_t*    _arena = nullptr;
        size_t      _capacity = 0;
        size_t      _offset = 0;

    public:
        ArenaAllocator() = delete;
        ArenaAllocator(const ArenaAllocator&) = delete;
        ArenaAllocator& operator=(const ArenaAllocator&) = delete;

        explicit ArenaAllocator(size_t capacity) {
            void* raw = ::operator new(capacity);

            _arena = static_cast<uint8_t*>(raw);
            _capacity = capacity;
        }

        ArenaAllocator(ArenaAllocator&& aa) noexcept {
            _arena = aa._arena;
            _capacity = aa._capacity;
            _offset = aa._offset;

            aa._arena = nullptr;
            aa._capacity = 0;
            aa._offset = 0;
        }

        ArenaAllocator& operator=(ArenaAllocator&& aa) noexcept {
            if (this == &aa) {
                return *this;
            }

            ::operator delete(_arena);
            _capacity = 0;
            _offset = 0;


            _arena = aa._arena;
            _capacity = aa._capacity;
            _offset = aa._offset;

            aa._arena = nullptr;
            aa._capacity = 0;
            aa._offset = 0;

            return *this;
        }

        ~ArenaAllocator() {
            ::operator delete(_arena);
        }

        void reset() {
            _offset = 0;
        }

        void* allocate(size_t size) {
            static constexpr size_t align = alignof(std::max_align_t);
             _offset = (_offset + (align - 1)) & ~(align - 1);

            if (_offset + size > _capacity) {
                throw std::bad_alloc();
            }

            void* ptr = _arena + _offset;
            _offset += size;
            return ptr;
        }

        template<typename T>
        T* allocate() {
            return static_cast<T*>(allocate(sizeof(T)));
        }

    };  //  class   ArenaAllocator
}   //  namespace   Hermes