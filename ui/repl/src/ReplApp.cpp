//  /ui/repl/src/ReplApp.cpp
#include "ReplApp.hpp"

#include "CommandDispatcher.hpp"
#include "synapse/SynapseEngine.hpp"

#include <iostream>

namespace Synapse::Repl {

    struct ReplApp::Impl {
        CommandDispatcher   _dispatcher;
        SynapseEngine       _engine;

        void _setupEngine();
        void _setupDispatcher();

        void _printHelp() const;
        void _printPlugins() const;
        void _printTools() const;
        void _printEnvironment() const;
    };  //  struct  ReplApp::Impl

    ReplApp::ReplApp() : _impl(new Impl) {}
    ReplApp::~ReplApp() = default;

    void ReplApp::run() {

    }
    

    //  --------------------
    //      IMPL METHODS    
    //  --------------------

    void ReplApp::Impl::_setupEngine() {

    }

    void ReplApp::Impl::_setupDispatcher() {

    }

    void ReplApp::Impl::_printHelp() const {

    }

    void ReplApp::Impl::_printPlugins() const {

    }

    void ReplApp::Impl::_printTools() const {

    }

    void ReplApp::Impl::_printEnvironment() const {

    }

}   //  namespace   Synapse::Repl
