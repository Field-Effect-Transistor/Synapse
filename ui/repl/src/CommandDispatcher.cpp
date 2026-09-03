//  /ui/repl/src/CommandDispatcher.cpp
#include "CommandDispatcher.hpp"

#include "StringUtils.hpp"
#include <stdexcept>

namespace Synapse::Repl {
    
    bool CommandDispatcher::dispatch(const std::string& input) const {
        size_t first_char = input.find_first_not_of("\t ");
        if (input[first_char] != ':' || first_char == std::string::npos) {
            return false;
        }

        auto args = splitArguments(input);
        if (args.empty()) {
            return false;
        }

        std::string current_path = args[0];
        for(size_t i = 1; i < args.size(); ++i) {
            current_path += " " + args[i];
        }

        std::string_view search_view(current_path);

        while(!search_view.empty()) {
            auto it = _handlers.find(std::string(search_view));
            if (it != _handlers.end()) {
                size_t words_in_cmd = 1;
                for (char c : search_view) if (c == ' ') words_in_cmd++;

                std::vector<std::string> remaining_args(args.begin() + words_in_cmd, args.end());
                
                it->second(remaining_args);
                return true;
            }

            size_t last_space = search_view.find_last_of(' ');
            if (last_space == std::string_view::npos) {
                break;
            }
            
            search_view = search_view.substr(0, last_space);
        }

        throw std::runtime_error("Unknown command. Type :help for the list of commands.");
    }

    CommandDispatcher& CommandDispatcher::registerCommand(const std::string& cmd_path, Handler handler) {
        _handlers[cmd_path] = std::move(handler);
        return *this;
    }

}   //  namespace   Synapse::Repl
