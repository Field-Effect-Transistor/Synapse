//  /utils/include/Vector.hpp

#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <stdexcept>
#include <initializer_list>

namespace Synapse {

    template<typename T>
    class Vector {
        T* _data = nullptr;
        size_t _size = 0;
        size_t _capacity = 0;

    public:
        Vector() = default;
        ~Vector();

        Vector(const Vector& v);
        Vector(Vector&& v) noexcept;

        Vector& operator=(const Vector& v);
        Vector& operator=(Vector&& v) noexcept;

        Vector(std::initializer_list<T> init) : Vector() {
            reserve(init.size());
            for (const auto& m : init) {
                push_back(m);
            }
        }

        explicit Vector(size_t count);

        // ACCESS

        inline T* data() noexcept { return _data; }
        inline const T* data() const noexcept { return _data; }

        T& operator[](size_t index) { return _data[index]; };
        const T& operator[](size_t index) const { return _data[index]; };

        T& at(size_t index);
        const T& at(size_t index) const;

        inline T& front() noexcept { return _data[0]; }
        inline const T& front() const noexcept { return _data[0]; }

        inline T& back() noexcept { return _data[_size - 1]; }
        inline const T& back() const noexcept { return _data[_size - 1]; }

        // CAPACITY n SIZE

        inline size_t size() const noexcept { return _size; }
        inline size_t capacity() const noexcept { return _capacity; }
        inline bool empty() const noexcept { return _size == 0; }

        // MODIFIERS

        T& push_back(const T& value);
        T& push_back(T&& value);
        void pop_back() noexcept;
        void clear();

        // MEMORY MAN

        void reserve(size_t new_cap);
        void resize(size_t new_size);
        void shrink_to_fit();

        // Iterators

        T* begin() noexcept { return _data; }
        const T* begin() const noexcept { return _data; }

        T* end() noexcept { return _data + _size; }
        const T* end() const noexcept { return _data + _size; }

    };  //  class Vector


    template<typename T>
    Vector<T>::~Vector() {
        for(size_t i = 0; i < _size; ++i) {
            _data[i].~T();
        }
        ::operator delete(_data);
    }

    template<typename T>
    Vector<T>::Vector(const Vector& v) {
        size_t i = 0;
        void* raw = nullptr;
        T* data = nullptr;

        try {
            raw = ::operator new(v._capacity * sizeof(T));
            data = static_cast<T*>(raw);
            for (; i < v._size; ++i) {
                new(data + i) T(v._data[i]);
            }
        } catch (...) {
            for (size_t j = 0; j < i; ++j) {
                data[j].~T();
            }
            ::operator delete(raw);
            throw;
        }

        _size = v._size;
        _capacity = v._capacity;
        _data = data;
    }

    template<typename T>
    Vector<T>::Vector(Vector&& v) noexcept {
        _data = v._data;
        _size = v._size;
        _capacity = v._capacity;

        v._data = nullptr;
        v._size = 0;
        v._capacity = 0;
    }

    template<typename T>
    Vector<T>& Vector<T>::operator=(const Vector& v) {
        if(this == &v) {
            return *this;
        }

        size_t i = 0;
        void* raw = nullptr;
        T* data = nullptr;

        try {
            raw = ::operator new(v._capacity * sizeof(T));
            data = static_cast<T*>(raw);
            for(; i < v._size; ++i) {
                new(data + i) T(v._data[i]);
            }

            for (size_t j = 0; j < _size; ++j) {
                _data[j].~T();
            }
            ::operator delete(_data);

        } catch (...) {
            for (size_t j = 0; j < i; ++j) {
                data[j].~T();
            }
            ::operator delete(raw);
            throw;
        }

        _size = v._size;
        _capacity = v._capacity;
        _data = data;

        return *this;
    }

    template<typename T>
    Vector<T>& Vector<T>::operator=(Vector&& v) noexcept {
        if (this == &v) {
            return *this;
        }

        for(size_t i = 0; i < _size; ++i) {
            _data[i].~T();
        }
        ::operator delete(_data);

        _data = v._data;
        _size = v._size;
        _capacity = v._capacity;

        v._data = nullptr;
        v._size = 0;
        v._capacity = 0;

        return *this;
    }

    template<typename T>
    void Vector<T>::reserve(size_t new_cap) {
        if (new_cap == 0) {
            new_cap = 1;
        }
        if (_capacity >= new_cap) {
            return;
        }

        T* data = nullptr;
        size_t i = 0;
        try {
            data = static_cast<T*>(::operator new(new_cap * sizeof(T)));
            for(; i < _size; ++i) {
                new(data + i) T(std::move_if_noexcept(_data[i]));
            }
        } catch (...) {
            for(size_t j = 0; j < i; ++j) {
                data[j].~T();
            }
            ::operator delete(data);
            throw;
        }

        for(size_t j = 0; j < _size; ++j) {
            _data[j].~T();
        }
        ::operator delete(_data);

        _data = data;
        _capacity = new_cap;
    }

    template<typename T>
    void Vector<T>::clear() {
        for(size_t i = 0; i < _size; ++i) {
            _data[i].~T();
        }
        _size = 0;
    }

    template<typename T>
    T& Vector<T>::at(size_t index) {
        if (index >= _size) {
            throw std::out_of_range("Synapse::Vector::at(): out of vector range");
        }
        return _data[index];
    }

    template<typename T>
    const T& Vector<T>::at(size_t index) const {
        if (index >= _size) {
            throw std::out_of_range("Synapse::Vector::at() const: out of vector range");
        }
        return _data[index];
    }

    template<typename T>
    T& Vector<T>::push_back(const T& value) {
        if (_size >= _capacity) {
            reserve(2 * _capacity);
        }
        new(_data + _size) T(value);

        return _data[_size++];
    }

    template<typename T>
    T& Vector<T>::push_back(T&& value) {
        if (_size >= _capacity) {
            reserve(2 * _capacity);
        }
        new(_data + _size) T(std::move(value));

        return _data[_size++];
    }

    template<typename T>
    void Vector<T>::pop_back() noexcept {
        _data[--_size].~T();
    }

    template<typename T>
    void Vector<T>::resize(size_t new_size) {
        if (new_size < _size) {
            for (size_t i = new_size; i < _size; ++i) {
                _data[i].~T();
            }
        } else if (new_size > _size) {
            if (new_size > _capacity) {
                reserve(new_size);
            }
            for (size_t i = _size; i < new_size; ++i) {
                new(_data + i) T();
            }
        }
        _size = new_size;
    }

    template<typename T>
    void Vector<T>::shrink_to_fit() {
        if (_capacity == _size) {
            return;
        }

        if (_size == 0) {
            ::operator delete(_data);
            _data = nullptr;
            _capacity = 0;
            return;
        }

        T* new_data = nullptr;
        size_t i = 0;

        try {
            new_data = static_cast<T*>(::operator new(_size * sizeof(T)));
            
            for(; i < _size; ++i) {
                new(new_data + i) T(std::move_if_noexcept(_data[i]));
            }
        } catch (...) {
            for(size_t j = 0; j < i; ++j) {
                new_data[j].~T();
            }
            ::operator delete(new_data);
            throw;
        }

        for(size_t j = 0; j < _size; ++j) {
            _data[j].~T();
        }
        ::operator delete(_data);

        _data = new_data;
        _capacity = _size;
    }

    template<typename T>
    Vector<T>::Vector(size_t count) {
        if (count == 0) return;

        _capacity = count;
        _data = static_cast<T*>(::operator new(count * sizeof(T)));
        
        try {
            for (_size = 0; _size < count; ++_size) {
                new(_data + _size) T();
            }
        } catch (...) {
            for (size_t i = 0; i < _size; ++i) {
                _data[i].~T();
            }
            ::operator delete(_data);
            _data = nullptr;
            _size = 0;
            _capacity = 0;
            throw;
        }
    }

}   //  namespace   Synapse
