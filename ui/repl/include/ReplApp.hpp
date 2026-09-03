//  /ui/repl/include/ReplApp.hpp
#pragma once

#include "UniquePtr.hpp"

namespace Synapse::Repl {

    class ReplApp final {
        struct Impl;
        UniquePtr<Impl> _impl;

    public:
        ReplApp();
        ~ReplApp();

        ReplApp(const ReplApp&) = delete;
        ReplApp(ReplApp&&)      = delete;

        ReplApp& operator=(const ReplApp)   = delete;
        ReplApp& operator=(ReplApp&&)       = delete;

        void run();

    };  //  class   ReplApp

}   //  namespace   Synapse::Repl
