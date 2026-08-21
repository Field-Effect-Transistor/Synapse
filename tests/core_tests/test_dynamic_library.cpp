//  /tests/core_tests/test_dynamic_library.cpp
#include <gtest/gtest.h>
#include "synapse/DynamicLibrary.hpp"
#include "synapse/interface/IPlugin.hpp"

#include <stdexcept>

using namespace Synapse;

class DynamicLibraryTest : public ::testing::Test {
protected:
    const char* valid_plugin_path = PRINTER_LIB_PATH;
    const char* empty_plugin_path = EMPTY_LIB_PATH;
};  //  class   DynamicLibraryTest

using DynamicLibraryInitTest = DynamicLibraryTest;
using DynamicLibraryLoadTest = DynamicLibraryTest;
using DynamicLibraryMoveTest = DynamicLibraryTest;

//  --------------------
//      Initialization
//  --------------------

TEST_F(DynamicLibraryInitTest, ThrowsOnNullptrPath) {
    EXPECT_THROW(DynamicLibrary(nullptr), std::runtime_error);
}

TEST_F(DynamicLibraryInitTest, ThrowsOnInvalidPath) {
    EXPECT_THROW(DynamicLibrary("non_existent_library_abc123.so"), std::runtime_error);
}

//  --------------------
//      Loading
//  --------------------

TEST_F(DynamicLibraryLoadTest, LoadsValidPluginAndExtractsSymbol) {
    DynamicLibrary lib(valid_plugin_path);
    
    IPlugin::Ptr plugin = lib.getSymbol();
    
    ASSERT_NE(plugin.get(), nullptr);
    EXPECT_STREQ(plugin->getName(), "PrinterLib");
}

TEST_F(DynamicLibraryLoadTest, ThrowsWhenSymbolIsMissing) {
    DynamicLibrary lib(empty_plugin_path);
    EXPECT_THROW(lib.getSymbol(), std::runtime_error);
}

//  --------------------
//      Move Semantics
//  --------------------

TEST_F(DynamicLibraryMoveTest, MoveConstructorTransfersOwnership) {
    DynamicLibrary original(valid_plugin_path);
    DynamicLibrary moved(std::move(original));

    IPlugin::Ptr plugin = moved.getSymbol();
    EXPECT_NE(plugin.get(), nullptr);
}

TEST_F(DynamicLibraryMoveTest, MoveAssignmentTransfersOwnership) {
    DynamicLibrary original(valid_plugin_path);
    DynamicLibrary target(empty_plugin_path);

    target = std::move(original);

    IPlugin::Ptr plugin = target.getSymbol();
    EXPECT_NE(plugin.get(), nullptr);
}
