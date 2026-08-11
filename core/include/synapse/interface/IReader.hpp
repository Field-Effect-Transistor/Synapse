//  /core/include/synapse/interface/IReader.hpp
#pragma once
#include <cstddef>

namespace Synapse {
    struct IReader {
        virtual ~IReader() = default;
        
        virtual size_t read(char* buffer, size_t size) = 0;
        
        virtual bool isEOF() const = 0;
    };
}
