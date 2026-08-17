//  /tests/core_tests/test_plugin_registry.cpp
#include <gtest/gtest.h>
#include "synapse/PluginRegistry.hpp"
#include "synapse/ContextManager.hpp"
#include "synapse/Context.hpp"
#include "synapse/interface/ILexer.hpp"
#include "synapse/interface/IParser.hpp"

using namespace Synapse;

// =================
//      MOCKS
// =================

struct DummyLexer : public ILexer {
    void destroy() override { delete this; }
    void init(IReader*, size_t) override {} 
    Token getNextToken() override { return Token(); }
    const Token& peekToken() override { static Token t; return t; }
    bool isEOF() override { return true; }
};

struct DummyParser : public IParser {
    void destroy() override { delete this; }
    IASTNode::Ptr parse(const Vector<Token>&) override { return IASTNode::Ptr(nullptr); }
};

struct DummyVisitor : public IVisitor {
    void destroy() override { delete this; }
    Value visit(LiteralNode&) override { return Value(); }
    Value visit(VariableNode&) override { return Value(); }
    Value visit(BinaryNode&) override { return Value(); }
    Value visit(UnaryNode&) override { return Value(); }
    Value visit(FunctionNode&) override { return Value(); }
};

ILexer*     create_dummy_lexer() { return new DummyLexer(); }
IParser*    create_dummy_parser() { return new DummyParser(); }
IVisitor*   create_dummy_simple_visitor() { return new DummyVisitor(); }
IVisitor*   create_dummy_contextual_visitor(Context*) { return new DummyVisitor(); }

class MockPluginAlpha : public IPlugin {
public:
    void destroy() override { delete this; }
    const char* getName() const override { return "alpha"; }
    const char* getVersion() const override { return "1.0"; }
    const char* getDescription() const override { return "Mock Alpha Plugin"; }

    PluginManifest getManifest() const override {
        PluginManifest m;
        m.variables.push_back({"pi", "Pi constant", Value(3.14), true});
        
        m.lexers.push_back({"CommonLexer", "Alpha Common Lexer", &create_dummy_lexer});
        m.parsers.push_back({"AlphaParser", "Unique Alpha Parser", &create_dummy_parser});
        return m;
    }
};

class MockPluginBeta : public IPlugin {
public:
    void destroy() override { delete this; }
    const char* getName() const override { return "beta"; }
    const char* getVersion() const override { return "2.0"; }
    const char* getDescription() const override { return "Mock Beta Plugin"; }

    PluginManifest getManifest() const override {
        PluginManifest m;
        m.variables.push_back({"pi", "Bad Pi constant", Value(3.0), true});
        
        m.lexers.push_back({"CommonLexer", "Beta Common Lexer", &create_dummy_lexer});
        m.lexers.push_back({"UniqueLexer", "Unique Beta Lexer", &create_dummy_lexer});
        m.simple_visitors.push_back({"MyVisitor", "Test Simple Visitor", &create_dummy_simple_visitor});
        m.contextual_visitors.push_back({"MyCtxVisitor", "Test Contextual Visitor", &create_dummy_contextual_visitor});
        
        return m;
    }
};

class MockPluginEmpty : public IPlugin {
public:
    void destroy() override { delete this; }
    const char* getName() const override { return "empty"; }
    const char* getVersion() const override { return "0.0"; }
    const char* getDescription() const override { return "Empty Plugin"; }

    PluginManifest getManifest() const override {
        return PluginManifest{};
    }
};

IPlugin::Ptr make_plugin(IPlugin* p) {
    return IPlugin::Ptr(p);
}


// =================
//      TESTS
// =================

class PluginRegistryTest : public ::testing::Test {
protected:
    PluginRegistry registry;

    void loadAllMocks() {
        registry.loadPlugin(make_plugin(new MockPluginAlpha()));
        registry.loadPlugin(make_plugin(new MockPluginBeta()));
        registry.loadPlugin(make_plugin(new MockPluginEmpty()));
    }
};

using PluginRegistryLifecycleTest       = PluginRegistryTest;
using PluginRegistryExactMatchTest      = PluginRegistryTest;
using PluginRegistrySmartFallbackTest   = PluginRegistryTest;
using PluginRegistryContextTest         = PluginRegistryTest;

TEST_F(PluginRegistryLifecycleTest, LoadsPluginsAndReturnsMetadata) {
    loadAllMocks();
    auto plugins = registry.getLoadedPlugins();
    
    EXPECT_EQ(plugins.size(), 3);
    EXPECT_EQ(plugins[0].name, "alpha");
    EXPECT_EQ(plugins[1].version, "2.0");
    EXPECT_EQ(plugins[2].description, "Empty Plugin");
}

TEST_F(PluginRegistryLifecycleTest, DiscoveryApiReturnsCorrectTools) {
    loadAllMocks();
    auto lexers = registry.getAvailableLexers();
    
    EXPECT_EQ(lexers.size(), 3); // 1 від alpha, 2 від beta
    
    bool found_unique = false;
    for (const auto& l : lexers) {
        if (l.getFullName() == "beta.UniqueLexer") found_unique = true;
    }
    EXPECT_TRUE(found_unique);
}

TEST_F(PluginRegistryExactMatchTest, CreatesToolsWithExactMatch) {
    loadAllMocks();
    
    EXPECT_TRUE(registry.hasLexer("alpha", "CommonLexer"));
    EXPECT_TRUE(registry.hasLexer("beta", "CommonLexer"));

    // Смарт-поінтери самі видалять створені Dummy-об'єкти!
    ILexer::Ptr lex_a = registry.createLexer("alpha", "CommonLexer");
    ILexer::Ptr lex_b = registry.createLexer("beta", "CommonLexer");

    EXPECT_NE(lex_a.get(), nullptr);
    EXPECT_NE(lex_b.get(), nullptr);
}

TEST_F(PluginRegistryExactMatchTest, ThrowsWhenExactMatchNotFound) {
    loadAllMocks();
    EXPECT_FALSE(registry.hasParser("alpha", "UnknownParser"));
    EXPECT_THROW(registry.createParser("alpha", "UnknownParser"), std::runtime_error);
}

TEST_F(PluginRegistrySmartFallbackTest, SmartSearchResolvesUniqueTools) {
    loadAllMocks();
    
    // AlphaParser є тільки в одному плагіні, тому Ядро має його знайти без префікса
    EXPECT_TRUE(registry.hasParser("AlphaParser"));
    
    IParser::Ptr parser = registry.createParser("AlphaParser");
    EXPECT_NE(parser.get(), nullptr);
}

TEST_F(PluginRegistrySmartFallbackTest, SmartSearchThrowsOnAmbiguousNames) {
    loadAllMocks();
    
    // CommonLexer є і в Alpha, і в Beta. Розумний пошук має провалитися.
    EXPECT_FALSE(registry.hasLexer("CommonLexer"));
    EXPECT_THROW(registry.createLexer("CommonLexer"), std::runtime_error);
}

TEST_F(PluginRegistrySmartFallbackTest, SmartSearchWorksWithDotNotation) {
    loadAllMocks();
    
    // Перевіряємо, чи Smart пошук вміє розрізати рядок
    EXPECT_TRUE(registry.hasLexer("beta.UniqueLexer"));
    
    ILexer::Ptr lex = registry.createLexer("beta.UniqueLexer");
    EXPECT_NE(lex.get(), nullptr);
}

TEST_F(PluginRegistryContextTest, FillsMultipleContextsSafely) {
    registry.loadPlugin(make_plugin(new MockPluginAlpha()));

    ContextManager manager;
    Context* ctx1 = manager.createScope();
    Context* ctx2 = manager.createScope();

    // Заповнюємо обидва
    registry.fillContext(ctx1);
    registry.fillContext(ctx2);

    // Обидва мають отримати свої копії 'pi'
    EXPECT_DOUBLE_EQ(ctx1->getVariable("pi").getNumber(), 3.14);
    EXPECT_DOUBLE_EQ(ctx2->getVariable("pi").getNumber(), 3.14);
}

TEST_F(PluginRegistryContextTest, ThrowsOnVariableCollisionDuringFill) {
    // Вантажимо обидва плагіни (в обох є змінна "pi")
    loadAllMocks();

    ContextManager manager;
    Context* ctx = manager.createScope();

    // Спроба заповнити контекст має впасти, бо Context забороняє перезапис змінних в одному Scope
    EXPECT_THROW(registry.fillContext(ctx), std::runtime_error);
}

TEST_F(PluginRegistryContextTest, CreatesVisitors) {
    loadAllMocks(); 
    
    EXPECT_TRUE(registry.hasSimpleVisitor("beta", "MyVisitor"));
    EXPECT_NE(registry.createSimpleVisitor("beta", "MyVisitor").get(), nullptr);
    
    EXPECT_TRUE(registry.hasContextualVisitor("beta", "MyCtxVisitor"));
    EXPECT_NE(registry.createContextualVisitor("beta", "MyCtxVisitor", nullptr).get(), nullptr);
}

TEST_F(PluginRegistryTest, ThrowsOnDuplicateFactoryInManifest) {
    class BadPlugin : public MockPluginEmpty {
        const char* getName() const override { return "bad"; }
        PluginManifest getManifest() const override {
            PluginManifest m;
            // ДВА однакові інструменти в одному маніфесті!
            m.lexers.push_back({"Same", "", &create_dummy_lexer});
            m.lexers.push_back({"Same", "", &create_dummy_lexer});
            return m;
        }
    };
    EXPECT_THROW(registry.loadPlugin(make_plugin(new BadPlugin())), std::runtime_error);
}
