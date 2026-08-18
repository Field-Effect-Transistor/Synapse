// plugin/PrinterLib/src/PrinterLibPlugin.cpp
#include "synapse/interface/IPlugin.hpp"

#include "ASTPrinter.hpp"
#include "ASTTreePrinter.hpp"

namespace PrinterLib {

    IVisitor* create_flat_printer() { return new ASTPrinter; }
    IVisitor* create_tree_printer() { return new ASTTreePrinter; }

    class PrinterPlugin final : public Synapse::IPlugin {
    public:
        void destroy() override { delete this; }
        const char* getName() const override { return "PrinterLib"; }
        const char* getVersion() const override { return "1.0.0"; }
        const char* getDescription() const override { return "AST formatting and printing tools"; }

        Synapse::PluginManifest getManifest() const override {
            Synapse::PluginManifest manifest;
            
            manifest.simple_visitors.push_back({
                "ASTPrinter", 
                "Prints the AST as a flat LISP-like string", 
                &create_flat_printer
            });

            manifest.simple_visitors.push_back({
                "ASTTreePrinter", 
                "Prints the AST as a beautiful 2D tree", 
                &create_tree_printer
            });
            
            return manifest;
        }
    };

} // namespace PrinterLib

extern "C" {
    Synapse::IPlugin* create_plugin() {
        return new PrinterLib::PrinterPlugin();
    }
}
