//  /core/src/Token.cpp
#include "synapse/Token.hpp"

#include <iomanip>
#include <iostream>

namespace Synapse {
    void Token::print(std::ostream& os) const {
        std::string positionInfo = "[R:" + std::to_string(row) + " C:" + std::to_string(column) + "]";
        std::string lexemeInfo = "('" + lexeme + "')";

        os << std::left 
            << std::setw(14) << positionInfo
            << std::setw(16) << StandardToken::toString(type)
            << std::setw(25) << lexemeInfo;
        
        if (type == StandardToken::NUMBER) {
            os << " Val: " << value;
        }
    }
}
