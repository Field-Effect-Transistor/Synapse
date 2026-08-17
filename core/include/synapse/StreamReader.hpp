//  /core/include/synapse/StreamReader.hpp

#pragma once
#include "synapse/interface/IReader.hpp"
#include <istream>

namespace Synapse {

    class StreamReader final : public IReader {
        std::istream& _is;

    public:
        explicit StreamReader(std::istream& is) : _is(is) {}

        size_t read(char* buffer, size_t size) override {
            _is.read(buffer, static_cast<std::streamsize>(size));
            return static_cast<size_t>(_is.gcount());
        }

        bool isEOF() const override {
            return _is.eof();
        }
    };

} // namespace Synapse
