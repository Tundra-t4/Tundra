#include "base.hpp"
#include "interpreterc.hpp"
#include "Scope.hpp"
#include "Tokens.hpp"
#include "Lexer.hpp"
#include "AST.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "Instructions.hpp"
#include "SymbolTable.hpp"
#include "Macrophange.hpp"
#include <sys/mman.h>




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
    AP = ArgParser(argv,argc);
    AP.parse();
    if (argc < 2){
        std::cout << "Tundra c(ompiler) [v Arcadia]\nhttps://github.com/Tundra-t4/Tundra\n(Tip: run 'Tundrac help' for help)" << std::endl;
        exit(0);
    }
    if (AP.has("version")){
        std::cout << "Tundra c(ompiler): Anthology no. 2025b (Arcadia)" << std::endl;
        exit(0);
    }
    if (AP.has("-unbundle")){
        std::unordered_map<std::string,std::string> imports;
        std::vector<char> ObjContent;
        auto bundler = ObjectBundler();
        bundler.unbundle(argv[2],&imports,&ObjContent);
        for (auto& b : imports){
            std::cout << b.first << ":" << b.second << std::endl; 
        } 
        exit(0);
    }
    
    
    //std::cout << std::boolalpha;

    std::string inpfn(argv[1]);
    if (AP.has("compile")){
        inpfn = AP.values["compile"][0];
        println(green_block + circle + reset + " Beginning build.");
        ruleset["typing"] = "static";
    }
    if (AP.has("typing")){
        ruleset["typing"] = AP.values["typing"][0];
    } 

    std::ifstream file(inpfn);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << "\n";
        file.close();
        return 1;
    }
    

    std::string input((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    std::string builtins_str = "import {} from Builtins;";
    if (input.find("#(no_builtins)") != std::string::npos){
        builtins_str = "";
    }

    input = "" + builtins_str + input + "";


    Lexer lexer(input);
    file.close();
    Parser parser(lexer);


    auto rootBlock = parser.parse();
    if (!rootBlock) {
        std::cerr << "Parsing error\n";
        return 1;
    }
    It.rootblock = rootBlock;

    std::shared_ptr<SymbolTable> SymTab = std::make_shared<SymbolTable>();

    if (AP.has("compile")){
        SymTab->symbol(rootBlock);
        //println("Finished symbolification");
        
    } else {
    

        std::shared_ptr<Interpreter> interpreter = std::make_shared<Interpreter>();
        It.setinte(interpreter);
        try {
        
        interpreter->interpret(rootBlock);
        } catch(std::runtime_error& e){
            savelog();
            throw e;
        }
    }
    logat("Finished execution","m");
    savelog();
    if (AP.has("compile")){
    It.finish();
    }

    

    return 0;
}

