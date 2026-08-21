//  /plugin/PrinterLib/tests/test_plugin.cpp
#include <gtest/gtest.h>
#include "synapse/interface/IPlugin.hpp"
#include "synapse/interface/IVisitor.hpp"
#include "UniquePtr.hpp"

using namespace Synapse;

extern "C" Synapse::IPlugin* create_plugin();

class PrinterLibPluginTest : public ::testing::Test {
protected:
    IPlugin::Ptr plugin;

    void SetUp() override {
        plugin = IPlugin::Ptr(create_plugin());
    }
};  //  class   PrinterLibPluginTest

using PrinterLibMetadataTest  = PrinterLibPluginTest;
using PrinterLibManifestTest  = PrinterLibPluginTest;
using PrinterLibLifecycleTest = PrinterLibPluginTest;


//  ----------------
//      Metadata
//  ----------------

TEST_F(PrinterLibMetadataTest, ProvidesCorrectMetadata) {
    ASSERT_NE(plugin.get(), nullptr);

    EXPECT_STREQ(plugin->getName(), "PrinterLib");
    EXPECT_STREQ(plugin->getVersion(), "1.0.0");
    EXPECT_STREQ(plugin->getDescription(), "AST formatting and printing tools");
}


//  ----------------
//      Manifest
//  ----------------

TEST_F(PrinterLibManifestTest, ManifestContainsExpectedTools) {
    ASSERT_NE(plugin.get(), nullptr);
    PluginManifest manifest = plugin->getManifest();

    EXPECT_TRUE(manifest.lexers.empty());
    EXPECT_TRUE(manifest.parsers.empty());
    EXPECT_TRUE(manifest.variables.empty());
    EXPECT_TRUE(manifest.functions.empty());

    ASSERT_EQ(manifest.visitors.size(), 2);

    auto ast_printer_decl = manifest.visitors[0];
    EXPECT_STREQ(ast_printer_decl.name, "ASTPrinter");
    EXPECT_STREQ(ast_printer_decl.description, "Prints the AST as a flat LISP-like string");
    EXPECT_EQ(ast_printer_decl.role, IVisitor::Role::Producer);
    ASSERT_NE(ast_printer_decl.factory, nullptr);

    IVisitor::Ptr flat_printer(ast_printer_decl.factory(nullptr));
    EXPECT_NE(flat_printer.get(), nullptr);
    EXPECT_EQ(flat_printer->getRole(), IVisitor::Role::Producer);

    auto tree_printer_decl = manifest.visitors[1];
    EXPECT_STREQ(tree_printer_decl.name, "ASTTreePrinter");
    EXPECT_STREQ(tree_printer_decl.description, "Prints the AST as a beautiful 2D tree");
    EXPECT_EQ(tree_printer_decl.role, IVisitor::Role::Producer);
    ASSERT_NE(tree_printer_decl.factory, nullptr);

    IVisitor::Ptr tree_printer(tree_printer_decl.factory(nullptr));
    EXPECT_NE(tree_printer.get(), nullptr);
    EXPECT_EQ(tree_printer->getRole(), IVisitor::Role::Producer);
}


//  --------------------
//      Lifecycle
//  --------------------

TEST_F(PrinterLibLifecycleTest, DestroysProperly) {
    IPlugin* raw_plugin = create_plugin();
    ASSERT_NE(raw_plugin, nullptr);

    raw_plugin->destroy();
}
