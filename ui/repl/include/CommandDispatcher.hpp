//  /ui/repl/include/CommandDispatcher.hpp
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

namespace Synapse::Repl {


    class CommandDispatcher final {
    public:
        using Handler = std::function<void(const std::vector<std::string>& args)>;

    private:
        std::unordered_map<std::string, Handler>    _handlers;

    public:
        CommandDispatcher()     = default;
        ~CommandDispatcher()    = default;

        CommandDispatcher(const CommandDispatcher&) = delete;
        CommandDispatcher(CommandDispatcher&&)      = delete;

        CommandDispatcher& operator=(const CommandDispatcher&)  = delete;
        CommandDispatcher& operator=(CommandDispatcher&&)       = delete;


        bool dispatch(const std::string& input) const;
        CommandDispatcher& registerCommand(const std::string& cmd_path, Handler handler);

    };  //  class   CommandDispatcher

}   //  namespace   Synapse::Repl

