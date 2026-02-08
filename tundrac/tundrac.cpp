
#include "base.hpp"
#include "TypeSystem.hpp"
TypeRegistry& DTYR = TypeRegistry::instance();
tsl::ordered_map<std::string,std::shared_ptr<Instruction>> dummyinstructions = {{"none",std::make_shared<resource_instruction>(DTYR.get_void(),nullptr)},{"null",std::make_shared<resource_instruction>(DTYR.get_void(),nullptr)}};

#include "Tokens.hpp"
#include "Lexer.hpp"
#include "AST.hpp"

#include "Parser.hpp"
#include "SymbolTable.hpp"
#include "ScopeManager.hpp"

#include "Instructions.hpp"
#include "interpreterc.hpp"
#include "Macrophange.hpp"
#include <sys/mman.h>

// Forward declarations for macro support (kept for compatibility)
std::shared_ptr<ASTNode> Parser::invoke_macro(std::shared_ptr<MacroBlock> mB) {
    Macro Macrophange = Macro(this);
    Macrophange.setID(mB->MSID);
    return Macrophange.SubParse(mB);
}

std::shared_ptr<MacroInstruction> Parser::serial_macro() {
    Macro Macrophange = Macro(this);
    auto res = Macrophange.BeginExpansion();
    res->setMSID(Macrophange.id);
    return res;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    AP = ArgParser(argv, argc);
    AP.parse();
    
    if (argc < 2) {
        std::cout << "Tundra Compiler [v Arcadia Refactored]\n"
                  << "https://github.com/Tundra-t4/Tundra\n"
                  << "(Tip: run 'tundrac help' for help)" << std::endl;
        return 0;
    }
    
    if (AP.has("version")) {
        std::cout << "Tundra Compiler: Anthology no. 2025b (Arcadia Refactored)" << std::endl;
        return 0;
    }
    
    if (AP.has("-unbundle")) {
        std::unordered_map<std::string, std::string> imports;
        std::vector<char> obj_content;
        auto bundler = ObjectBundler();
        bundler.unbundle(argv[2], &imports, &obj_content);
        for (const auto& [key, value] : imports) {
            std::cout << key << ":" << value << std::endl;
        }
        return 0;
    }
    
    // Get compiler state
    auto& state = CompilerState::instance();
    
    // Determine input file
    std::string input_file(argv[1]);
    if (AP.has("compile")) {
        input_file = AP.values["compile"][0];
        std::cout << Colors::GREEN << Symbols::CIRCLE << Colors::RESET 
                  << " Beginning build." << std::endl;
        state.typing_rule = "static";
    }
    
    if (AP.has("typing")) {
        state.typing_rule = AP.values["typing"][0];
    }
    
    // Read source file
    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << input_file << "\n";
        return 1;
    }
    
    std::string source_code(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    file.close();
    
    // Add built-ins unless disabled
    std::string builtins_import = "import {} from Builtins;";
    if (source_code.find("#(no_builtins)") != std::string::npos) {
        builtins_import = "";
    }
    source_code = builtins_import + source_code;
    
    // ========================================================================
    // COMPILATION PIPELINE
    // ========================================================================
    
    try {
        // Phase 1: Lexical Analysis
        logat("Starting lexical analysis", "main");
        Lexer lexer(source_code);
        
        // Phase 2: Syntax Analysis (Parsing)
        logat("Starting syntax analysis", "main");
        Parser parser(lexer);
        auto root_block = parser.parse();
        
        if (!root_block) {
            std::cerr << "Parsing failed\n";
            return 1;
        }
        
        // Phase 3: Semantic Analysis (Symbol Table Construction & Type Checking)
        logat("Starting semantic analysis", "main");
        auto symbol_table = std::make_unique<SymbolTable>();
        symbol_table->analyze(root_block);
        
        if (symbol_table->has_errors()) {
            std::cerr << Colors::RED << "Semantic analysis failed with errors:" 
                      << Colors::RESET << std::endl;
            symbol_table->print_errors();
            return 1;
        }
        
        logat("Semantic analysis completed successfully", "main");
        std::cout << Colors::GREEN << "✓ " << Colors::RESET 
                  << "Semantic analysis finished" << std::endl;
        
        // Phase 4: Code Generation (if compiling)
        if (AP.has("compile")) {
            logat("Starting code generation", "main");
            
            // Set up code generation context
            It.rootblock = root_block;
            
            // The existing polaroid-based code generation can be kept
            // but now it works with a properly analyzed AST
            
            // Finish code generation
            It.finish();
            
            std::cout << Colors::GREEN << "✓ " << Colors::RESET 
                      << "Compilation completed successfully" << std::endl;
        } else {

        }
        
    } catch (const std::exception& e) {
        std::cerr << Colors::RED << "Fatal error: " << e.what() 
                  << Colors::RESET << std::endl;
        if (state.debug_mode) {
            std::ofstream log_file("Tundra.log");
            log_file << state.log_content << "\nFatal error: " << e.what();
        }
        return 1;
    }
    
    logat("Finished execution", "main");
    
    // Save debug log if enabled
    if (state.debug_mode) {
        std::ofstream log_file("Tundra.log");
        log_file << state.log_content << "Execution completed successfully!";
    }
    
    return 0;
}