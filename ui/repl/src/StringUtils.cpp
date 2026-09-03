//  /ui/repl/src/StringUtils.cpp
#include "StringUtils.hpp"

namespace Synapse::Repl {

    std::vector<std::string> splitArguments(const std::string& input) {
        bool in_quotes = false;
        std::string curr_word;
        std::vector<std::string> res;
        res.reserve(4);

        for (size_t i = 0; i < input.size(); ++i) {
            switch (input[i]) {
                case '\"': {
                    in_quotes = !in_quotes;
                    break;
                }
                case ' ':
                case '\t':
                case '\n': {
                    if (in_quotes) {
                        curr_word += ' ';
                    } else {
                        if(!curr_word.empty()) {
                            res.push_back(curr_word);
                            curr_word.clear();
                        }
                    }
                    break;
                }
                default: {
                    curr_word += input[i];
                }
            }
        }

        if (!curr_word.empty()) {
            res.push_back(curr_word);
        }

        return res;
    }

}   //  Synapse::Repl
