#include "base.hpp"
#include "interpreterc.hpp"
#include "Scope.hpp"
#include "Tokens.hpp"
#include "AST.hpp"
#include "Lexer.hpp"
#include "Macro_Types.hpp"



#ifndef PARSER
#define PARSER
// Pretty self explanatory to be a Parser
class Parser {
public:
    Parser(Lexer& lexer) : lexer(lexer) {
        
        ////println(currentToken.value);
        currentToken = lexer.getNextToken();  // Get the first token
    }

    // Parse the entire input and return the root AST node
    std::shared_ptr<BlockNode> parse() {
        return parseBlock(true);
    }

    std::shared_ptr<ASTNode> invoke_macro(std::shared_ptr<MacroBlock> mB);

    std::shared_ptr<MacroInstruction> serial_macro();

    Lexer& lexer;
    Token currentToken;
    tsl::ordered_map<std::string,int> structs;
    tsl::ordered_map<std::string,std::shared_ptr<MacroInstruction>> macros;
    bool activeMacro = false;
    
    tsl::ordered_map<std::string,std::shared_ptr<std::shared_ptr<ASTNode>>> macroVars = {};

    void addMacroVars(std::string key,std::shared_ptr<std::shared_ptr<ASTNode>> val){
        macroVars[key] = val;
    }

    



    void clearMacroVars(){
        macroVars.clear();
    }


    void advance() {
        currentToken = lexer.getNextToken();
    }
    void rb(int pos){
        
        lexer.rollback(pos-1);
        currentToken = lexer.getNextToken();
        logat("Rolldback to " + currentToken.value,"P.r");

    }

    Token peek(int tokensahead){
        auto res = lexer.peek(tokensahead);
        currentToken = lexer.getNextToken();
        logat("peekd and returned to " + currentToken.value,"P.r");
        return res;

    }


    bool match(TokenType expected) {
        if (currentToken.type == expected) {
            advance();
            return true;
        }
        displayError("Expected '" + tokenTypeToString(expected)   +  + "' - Got: '" + currentToken.value + "'",lexer.line,lexer.prevposwline+1,lexer.poswline+1,gerr({"note","Please refer to the documentation for how to implement your program"}) );
        
        return false;
    }


    void error(const std::string& msg) {
        throw std::runtime_error("Parsing error: " + msg);
    }

    // Parse a block (i.e., {...})
    std::shared_ptr<BlockNode> parseBlock(bool ismain = false) {
        if (!ismain){
        if (!match(TokenType::LBrace)) {
            
        }
        }

        auto block = std::make_shared<BlockNode>();
        while (currentToken.type != TokenType::RBrace && currentToken.type != TokenType::EndOfFile) {
            block->addStatement(parseStatement());
        }
        if (!ismain){

        if (!match(TokenType::RBrace)) {
            
        }
        }
        return block;
    }


    std::shared_ptr<ASTNode> parseConvention(){
        advance(); // convention
        std::string mrule = std::dynamic_pointer_cast<StringLiteralNode>(parseStringLiteral())->getValue();
        std::vector<std::shared_ptr<ASTNode>> publications;
        match(TokenType::LBrace);
        while (currentToken.type != TokenType::RBrace){
            publications.push_back(parseStatement());
        }

        return std::make_shared<ConventionNode>(mrule,publications);


    }
    std::shared_ptr<ASTNode> parseExtern(){
        advance(); // extern
        auto apos = lexer.poswline;
        auto lang = consume(TokenType::Identifier);
        if (lang == "C"){
            std::vector<std::shared_ptr<ASTNode>> fns;
            std::vector<std::string> fnnames;
            std::vector<std::string> isvariadic;
            match(TokenType::LBrace);
            while (currentToken.type != TokenType::RBrace){
                auto s = currentToken.value;
                if (s != "fn"){
                    displayError("Expected 'fn' keyword in extern decleration",lexer.line,lexer.poswline,lexer.poswline+ s.size(),{});
                }
                match(TokenType::KeyWord); // fn

                auto fnname= currentToken.value;
                fnnames.push_back(fnname);
                advance(); // foo
                match(TokenType::LParen); // (
                tsl::ordered_map<std::string, std::shared_ptr<ASTNode>> args;
                tsl::ordered_map<int,std::string> tick;
                tsl::ordered_map<std::string,std::string> internals;
                int a = 0;
                while (currentToken.type != TokenType::RParen){
                    if (currentToken.type != TokenType::Variadic){

                    auto aid = consume(TokenType::Identifier);
                    match(TokenType::Colon);
                    
                    args[aid] = parseExpression();
                    tick[a] = aid;
                    a += 1;
                    } else {
                        advance(); // ...
                        isvariadic.push_back(fnname);
                        if (currentToken.type != TokenType::RParen){
                            auto bpos = lexer.poswline;
                            auto ctoken = currentToken.value;
                            advance();
                            tsl::ordered_map<std::string,std::string> notes;
                            notes["note"] = "A variadic operator (...) signifies the end of the function decleration";
                            displayError("Following the variadic operator (...) must be ')'\nGot: '" + ctoken + "'" ,lexer.line,bpos,lexer.poswline,notes);
                        }

                    }
                    if (currentToken.type != TokenType::RParen){
                        match(TokenType::COMMA);
                    }

                }
                match(TokenType::RParen);
                if (currentToken.type != TokenType::ReturnTy){
                    tsl::ordered_map<std::string,std::string> notes;
                    notes["note"] = "Tundra does not allow 'void' types to be returned therefore a specified return type is required";
                    displayError("Expected '->' in extern function decleration",lexer.line,lexer.poswline,lexer.poswline+2,notes);
                }
                match(TokenType::ReturnTy);
                args["-!retty"] = parseExpression();

                fns.push_back(std::make_shared<MappedFunctionNode>(args,std::make_shared<ASTNode>(),tick,internals));


            }
            advance(); // }
            return std::make_shared<ExternNode>(fns,fnnames,isvariadic);
        } else {
            displayError("Unsupported extern, currently supported externs are: C",lexer.line,apos,lexer.poswline,{});
        }
    }
    std::shared_ptr<ASTNode> parseFunction(){
        bool anon;
        if (currentToken.type == TokenType::Anon){
            anon = true;
        }
        advance(); // fn/||
        std::string fnname;
        if (!anon){
        fnname = consume(TokenType::Identifier);
        }
        consume(TokenType::LParen); // (
        bool map = ruleset["typing"] == "static";
        bool isselfptr = false;
        std::shared_ptr<ASTNode> retty;
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> args;
        tsl::ordered_map<int, std::string> tick;
        tsl::ordered_map<std::string, std::string> internals;
        tsl::ordered_map<int,std::string> argus;
        int a = 0;
        while (currentToken.type != TokenType::RParen){
            std::string name;
            if (currentToken.value == "@|" || currentToken.value == "@"){
                auto res = parseExpression();
                if (auto inte = std::dynamic_pointer_cast<InternalsNode>(res)){
                    name = inte->getPub();
                    internals[name] = inte->getValue();
                } else {
                    throw std::runtime_error("Unsupported Decree.");
                }
            } else {
                if (currentToken.type == TokenType::Ampersand && a == 0){
                    advance();
                    auto apos = lexer.poswline;
                    name = consume(TokenType::Identifier);
                    if (name != "self"){
                        displayError("Only self may start with an ampersand(&), no other identifier may.",lexer.line,apos,lexer.poswline,gerr({"note","Did you mean to put self? If not:","[O]","fn foo(arg:&type)...","[X]","fn foo(&arg)..."}));

                    } else {
                        isselfptr = true;
                    }
                } else {
                    name = consume(TokenType::Identifier); // arg
                }
            }
            if ((currentToken.type == TokenType::Colon || ruleset["typing"] == "static") && a == 0){
                if (name != "self"){

                
                match(TokenType::Colon); // :
                map = true;
                args[name] = parseExpression();
                tick[a] = name;
                } else {
                    map = true;
                    args[name] = std::shared_ptr<ASTNode>();
                    tick[a] = name;
                }



            }

            if (map == true && a != 0){
                consume(TokenType::Colon);
                args[name]= parseExpression();
                tick[a] = name;
            } else if (map == false && a != 0){
                argus[a] = name; // unmapped
            }
            if (currentToken.type != TokenType::RParen){

                
                consume(TokenType::COMMA);
            }

            
            a += 1;
        }
        advance(); // )
        if (ruleset["typing"] == "static"){
            map = true;
        }
        // args r done now
        if (map == true){
            if (currentToken.type != TokenType::ReturnTy){
                retty = std::make_shared<IdentifierNode>("none");
            } else {
            consume(TokenType::ReturnTy);
            retty = parseExpression();
            }
            args["-!retty"] = retty;
            if (currentToken.type == TokenType::LBrace){
            if (anon){
                return std::make_shared<MappedFunctionNode>(args,parseBlock(),tick,internals,isselfptr);
            }
            return std::make_shared<AssignNode>(fnname,std::make_shared<MappedFunctionNode>(args,parseBlock(),tick,internals,isselfptr));
            } else {
                if (anon){
                    return std::make_shared<MappedFunctionNode>(args,std::make_shared<BlockNode>(),tick,internals,isselfptr);
                }
                return std::make_shared<AssignNode>(fnname,std::make_shared<MappedFunctionNode>(args,std::make_shared<BlockNode>(),tick,internals,isselfptr));
            }
        }
        // all that's left is the noding then interpretations
        if (anon){
            return std::make_shared<UnMappedFunctionNode>(argus,parseBlock());
        }

        return std::make_shared<AssignNode>(fnname,std::make_shared<UnMappedFunctionNode>(argus,parseBlock()));
        
    }
    std::shared_ptr<ASTNode> parseSizeOf(){
        advance(); // szof/sizeof
        return std::make_shared<SizeOfNode>(parseExpression());
    }
    std::shared_ptr<ASTNode> parseStackSize(){
        advance(); // StackSize
        return std::make_shared<StackSizeNode>();
    }
    std::shared_ptr<ASTNode> parseP2I(){
        advance(); // pti/ptrtoint
        return std::make_shared<PtrtointNode>(parseExpression());
    }
    std::shared_ptr<ASTNode> parseTypeid(){
        advance(); // szof/sizeof
        return std::make_shared<TypeIDNode>(parseExpression());
    }
    std::shared_ptr<ASTNode> parseDecorator(){
        advance(); // @
        if (currentToken.value == "sig" || currentToken.value == "signature" ){
            return parseSignature();
        }
        if (currentToken.value == "unsafe" || currentToken.value == "usafe" ){
            return parseUnsafe();
        }
        if (currentToken.value == "safe"){
            return parseSafe();
        }
        if (currentToken.value == "i" || currentToken.value == "internal" ){
            return parseInternal();
        }
        if (currentToken.value == "szof" || currentToken.value == "sizeof" ){
            return parseSizeOf(); // uint64_t TypeSize = DL.getTypeAllocSize(LLVMType); // llvm::DataLayout DL(M);
        }
        if (currentToken.value == "tyid" || currentToken.value == "typeid" ){
            return parseTypeid();
        }
        if (currentToken.value == "pti" || currentToken.value == "ptrtoint"  || currentToken.value == "addressof"){
            return parseP2I();
        }
        if (currentToken.value == "StackSize"){
            return parseStackSize();
        }
        auto clr = parseExpression();
        auto fn = parseStatement();
        return std::make_shared<DecoratorNode>(clr,fn);
        // node time!
    }
    std::shared_ptr<ASTNode> ParseRet(){
        int l = lexer.line;
        advance(); // ret
        std::shared_ptr<ASTNode> expr;
        
        if (lexer.line == l){
            
            expr = parseExpression();
        }
        return std::make_shared<RetNode>(expr);
    }
    // Parse a statement, which can be an expression or a block
    std::shared_ptr<ASTNode> parseHashTag(){
        match(TokenType::Hashtag); // #
        auto idpos = lexer.poswline;
        std::string id;
        if (currentToken.type == TokenType::LParen){
            advance(); // (
            id = consume(TokenType::Identifier);
            consume(TokenType::RParen);

        } else {
            id = consume(TokenType::Identifier);
        }
        
        if (id == "ruleset"){
            match(TokenType::LParen);
            auto rid = consume(TokenType::Identifier);
            match(TokenType::Assign);
            int apos = lexer.poswline;
            auto exp = parseExpression();
            if (auto expn = std::dynamic_pointer_cast<StringLiteralNode>(exp)){
                ruleset[rid] = expn->getValue();
                
            } else {
                displayError("Ruleset expression must be a string",lexer.line,apos,lexer.poswline,gerr({"note","ruleset(typing=...) can only be declared as a string","[X]","ruleset(typing=...)","[O]","ruleset(typing=\"static\") or ruleset(typing=\"dynamic\")"}) );
            }
            
            
            
            match(TokenType::RParen);
            

        } else if (id == "link"){
            match(TokenType::LParen);
            auto rid = consume(TokenType::Identifier);
            match(TokenType::Assign);
            int apos = lexer.poswline;
            auto exp = parseExpression();
            if (auto expn = std::dynamic_pointer_cast<StringLiteralNode>(exp)){
                if (AP.values.find("link") == AP.values.end()){
                    AP.values["link"] = {};
                }
                AP.values["link"].push_back(expn->getValue());
                
            } else {
                displayError("Link expression must be a string",lexer.line,apos,lexer.poswline,gerr({"note","link(path=...) can only be declared as a string","[X]","link(path=...)","[O]","link(path=\"/Users/username/Documents/x.lib\")"}) );
            }
            
            
            
            match(TokenType::RParen);
            

        } else if (id == "linklib"){
            match(TokenType::LParen);
            auto rid = consume(TokenType::Identifier);
            match(TokenType::Assign);
            int apos = lexer.poswline;
            auto exp = parseExpression();
            if (auto expn = std::dynamic_pointer_cast<StringLiteralNode>(exp)){
                if (AP.values.find("linklib") == AP.values.end()){
                    AP.values["linklib"] = {};
                }
                AP.values["linklib"].push_back(expn->getValue());
                
            } else {
                displayError("Linklib expression must be a string",lexer.line,apos,lexer.poswline,gerr({"note","linklib(name=...) can only be declared as a string","[X]","link(name=...)","[O]","link(path=\"libm\") or link(name=\"m\")"}) );
            }
            
            
            
            match(TokenType::RParen);
            

        } else if (id == "result"){
            match(TokenType::LParen);
            auto rid = consume(TokenType::Identifier);
            result = rid;
            
            
            
            match(TokenType::RParen);
            

        } else if (id == "no_builtins"){
            // just for the check so no error
        } else if (id == "allow"){
            consume(TokenType::LParen);
            while (currentToken.type != TokenType::RParen){
                allowances.push_back(consume(TokenType::Identifier));
                if (currentToken.type != TokenType::RParen){
                    consume(TokenType::COMMA);
                }
            }
            consume(TokenType::RParen);
        }else {
            displayError("Invalid identifier for '#'\nGot: " + id,lexer.line,idpos,lexer.poswline-2,gerr({"note","Structure of '#' is #<id>(<key>=<type>) or #(<id>)","supported <id>s","\n- ruleset"}) );
        }
        
        auto austin = std::make_shared<ASTNode>(); isbadnode.push_back(austin->id); return austin;
    }
    std::shared_ptr<ASTNode> parseAssert(){
        advance(); // assert
        return std::make_shared<AssertionNode>(parseExpression());
    }
    std::shared_ptr<ASTNode> parseMod(){
        advance(); // mod
        auto id = parseIdentifier(); // must be id
        match(TokenType::LBrace);
        std::vector<std::shared_ptr<ASTNode>> states;
        while (currentToken.type != TokenType::RBrace){
            states.push_back(parseStatement());
        }
        match(TokenType::RBrace);
        return std::make_shared<ModNode>(id,states);
    }
    std::shared_ptr<ASTNode> parseTodo(){
        advance(); // todo
        match(TokenType::LBrace);
        int a = 0;
        while (currentToken.type != TokenType::RBrace || a != 0){
            if (currentToken.type == TokenType::LBrace){
                a += 1;
            } else if (currentToken.type == TokenType::RBrace){
                a -= 1;
            }
            advance();
        }
        match(TokenType::RBrace);
        auto austin = std::make_shared<ASTNode>(); isbadnode.push_back(austin->id); return austin;
    }
    std::shared_ptr<ASTNode> parseDrop(){
        advance(); // drop
        return std::make_shared<DropNode>(parseExpression());
    }
    std::shared_ptr<ASTNode> parseEnum(){
        tsl::ordered_map<std::string,std::vector<std::shared_ptr<ASTNode>>> ret;
        advance(); // enum
        auto enname = consume(TokenType::Identifier);
        match(TokenType::LBrace);
        while (currentToken.type != TokenType::RBrace){
            auto id = consume(TokenType::Identifier);
            ret[id] = {};
            if (currentToken.type == TokenType::LParen){
                match(TokenType::LParen);
                while (currentToken.type != TokenType::RParen){
                ret[id].push_back(parseExpression());
                if (currentToken.type != TokenType::RParen){
                    match(TokenType::COMMA);
                }
                }
                match(TokenType::RParen);
                
            }
            else {
                
            }
            if (currentToken.type != TokenType::RBrace){
                match(TokenType::COMMA);
            }
        }
        match(TokenType::RBrace);
        return std::make_shared<AssignNode>(enname,std::make_shared<EnumNode>(ret));
    }
    std::shared_ptr<ASTNode> parseMatch(){

        advance(); // match
        auto expr = parseExpression();
        tsl::ordered_map<std::shared_ptr<ASTNode>,std::shared_ptr<ASTNode>> nodesbodies;
        std::shared_ptr<ASTNode> elser;
        match(TokenType::LBrace);
        while (currentToken.type != TokenType::RBrace){
            if (currentToken.value == "_"){
                advance(); // _
                match(TokenType::EqArrow);
                elser = parseExpression();
            } else {
            auto expr1 = parseExpression();
            match(TokenType::EqArrow);
            nodesbodies[expr1] = parseBlock();
            }
        }
        match(TokenType::RBrace);
        return std::make_shared<MatchNode>(expr,nodesbodies,elser);
    }
    std::shared_ptr<ASTNode> parseClass(){
        advance(); // class
        auto n = consume(TokenType::Identifier); // x
        match(TokenType::LBrace);
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> cons;
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> fns;
        while (currentToken.type != TokenType::RBrace){
            auto stat = parseStatement();
            if (auto aa = std::dynamic_pointer_cast<AssignNode>(stat)){
                if (auto fn = std::dynamic_pointer_cast<MappedFunctionNode>(aa->getValue())){
                    fns[aa->getVarName()] = fn;
                }
                cons[aa->getVarName()] = aa->getValue();
            } else {
                throw std::runtime_error("smth");
            }

        }
        match(TokenType::RBrace);
        return std::make_shared<AssignNode>(n,(std::make_shared<ClassDeclNode>(fns,cons)));

    }
    std::shared_ptr<ASTNode> parsePub(){
        advance(); // pub/public
        auto exp = parseStatement();
        return std::make_shared<PubNode>(exp);

    }
    std::shared_ptr<ASTNode> parseInline(){
        advance(); // inline
        if (currentToken.value == "asm"){
            advance(); // asm
            consume(TokenType::LParen);
            std::string ASMStr = std::dynamic_pointer_cast<StringLiteralNode>(parseStringLiteral())->getValue();
            if (currentToken.type == TokenType::COMMA){
                advance(); // ,
            }
            std::vector<std::string> regs;
            std::unordered_map<std::string,std::shared_ptr<ASTNode>> in;
            std::unordered_map<std::string,std::shared_ptr<ASTNode>> out;
            std::unordered_map<std::string,std::shared_ptr<ASTNode>> inout;
            std::unordered_map<std::string,std::shared_ptr<ASTNode>> assigns;
            while (currentToken.type != TokenType::RParen){
                if (currentToken.value == "in"){
                    advance(); // in
                    consume(TokenType::LParen);
                    auto id = consume(TokenType::Identifier);
                    in[id] = std::make_shared<IdentifierNode>(id);
                    regs.push_back(id);
                    consume(TokenType::RParen);
                } else if (currentToken.value == "out"){
                    advance(); // out
                    consume(TokenType::LParen);
                    std::string id = consume(TokenType::Identifier);
                    out[id] = std::make_shared<StringLiteralNode>("outasm");
                    regs.push_back(id);
                    consume(TokenType::RParen);
                    consume(TokenType::KeyWord); // as (verify this better)
                    assigns[id] = parseExpression();
                } else if (currentToken.value == "inout"){
                    advance(); // inout
                    consume(TokenType::LParen);
                    std::string id = consume(TokenType::Identifier);

                    consume(TokenType::RParen);
                    if (currentToken.type != TokenType::EqArrow){
                    inout[id] = std::make_shared<IdentifierNode>(id);
                    regs.push_back(id);
                    assigns[id] = std::make_shared<IdentifierNode>(id);
                    } else {
                    advance(); // =>
                    auto nid = consume(TokenType::Identifier);
                    in[id] = std::make_shared<IdentifierNode>(id);
                    out[nid] = std::make_shared<IdentifierNode>(id);
                    regs.push_back(id);
                    regs.push_back(nid);
                    assigns[id] = std::make_shared<IdentifierNode>(id);
                    assigns[nid] = std::make_shared<IdentifierNode>(id);
                    }
                }
                if (currentToken.type != TokenType::RParen){
                    consume(TokenType::COMMA);
                }
            }
            consume(TokenType::RParen);
            return std::make_shared<ASMNode>(ASMStr,regs,in,out,inout,assigns);
        }
    }
    std::shared_ptr<ASTNode> symbolize(SymbolEntry symbol){
        try {
        if (symbol.dataType == "unknown"){
            return nullptr;
        }
        if (symbol.dataType == "RawString"){
            return std::make_shared<StringLiteralNode>(symbol.value);
        } else if (symbol.dataType == "i32"){
            return std::make_shared<IntLiteralNode>(symbol.value);
        } else if (converters.find(symbol.dataType) != converters.end()) {
            return std::make_shared<ONode>(Object(convertToDouble(symbol.value),symbol.dataType));
        } else if (dummyValues.find(symbol.dataType) != dummyValues.end()){
            return std::make_shared<Fakepass>(dummyValues[symbol.dataType]);
        } else {
            return std::make_shared<IdentifierNode>(symbol.dataType);
        }
        } catch  (...) {
            return nullptr;
        }
    }
    std::shared_ptr<ASTNode> parseImport(){
        advance(); // import
        if (currentToken.value == "header"){
        advance(); // header
        std::string fp = std::dynamic_pointer_cast<StringLiteralNode>(parseStringLiteral())->getValue();
        std::vector<std::shared_ptr<ASTNode>> vec;
        std::string dp;
        if (AP.has("headerpath")){
        for (auto& p : AP.values["headerpath"] ){
            if (std::filesystem::exists(p + "/" + fp)){
                dp = p;
                break;
            }
        }
        } else {
            dp = "./externals";
        }
        auto parser = CppHeaderParser();
        auto parsed = parser.parseFile(dp + "/" + fp,"");
        std::vector<std::shared_ptr<ASTNode>> externs;
        std::vector<std::string> fnnames;
        for (auto& sym: parsed){
            if (sym.type == SymbolType::Assignment){
                auto s = symbolize(sym);
                if (s){
                vec.push_back(std::make_shared<AssignNode>(sym.name,s,true)); 
                }
            } else if (sym.type == SymbolType::Macro) {
                auto s = symbolize(sym);
                if (s){
                vec.push_back(std::make_shared<AssignNode>(sym.name,s,true)); 
                }
            }  else if (sym.type == SymbolType::Function) {
                tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> argur;
                tsl::ordered_map<int,std::string> tick;
                argur["-!retty"] = std::make_shared<IdentifierNode>(sym.args[0]);
                for (int i = 1; i != sym.args.size();i++){

                    std::string idn = sym.args[i+1];
                    std::shared_ptr<ASTNode> an = std::make_shared<IdentifierNode>(idn);
                    argur[sym.args[i]] = an;
                    tick[i] = sym.args[i];
                    i++;
                }
                tsl::ordered_map<std::string, std::string> internals;
                externs.push_back(std::make_shared<MappedFunctionNode>(argur,std::make_shared<BlockNode>(),tick,internals,false));
                fnnames.push_back(sym.name);
            } else if (sym.type == SymbolType::Struct) {
                tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> fields;
                tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> cons;
                for (auto& s : sym.symbols){
                    auto sy = symbolize(sym);
                    if (sy){
                    fields[sym.name] = sy;
                    }
                }
                vec.push_back(std::make_shared<AssignNode>(sym.name,std::make_shared<StructDeclNode>(fields,cons),true));
            }
        }
        std::vector<std::string> variadic{};
        vec.push_back(std::make_shared<ExternNode>(externs,fnnames,variadic));
        return std::make_shared<ChainNode>(vec);
        } else {
            std::string imp;
            std::vector<std::string> from = {};
            if (currentToken.type == TokenType::LBrace){
                advance(); // {
                while (currentToken.type != TokenType::RBrace){
                    if (currentToken.type == TokenType::Variadic){
                        advance();
                        from.push_back("*");
                        break;

                    }
                    from.push_back(consume(TokenType::Identifier)); // x
                    if (currentToken.type!= TokenType::RBrace){
                        consume(TokenType::COMMA); // ,
                    }
                }
                consume(TokenType::RBrace); // }
                if (from.size() == 0){
                    from.push_back("*");
                }
                if (currentToken.value != "from"){
                    // error here throw std::runtime_error
                }
                advance(); // from
            }
            if (currentToken.type == TokenType::Identifier){
                imp = std::dynamic_pointer_cast<IdentifierNode>(parseIdentifier())->getValue();
            } else if (currentToken.type == TokenType::String){
                imp = std::dynamic_pointer_cast<StringLiteralNode>(parseStringLiteral())->getValue();
            } 
            auto n = imp;
            auto aas = split(n,'/');
            auto iname = aas[aas.size()-1];
            return std::make_shared<AssignNode>(n,std::make_shared<ImportNode>(imp,from));
        }
    }
    std::shared_ptr<ASTNode> parseTag(){
        auto pv = parseFunction();
        auto mfn = std::dynamic_pointer_cast<AssignNode>(pv);
        return std::make_shared<AssignNode>(mfn->getVarName(),std::make_shared<TagNode>(mfn->getValue()),mfn->getMut());
        
    }
    std::shared_ptr<ASTNode> parseStatement() {

        std::shared_ptr<ASTNode> state;
        logat("Parsing next statement","Parser.pS");
        auto apos = lexer.prevposwline;
        auto aline = lexer.line;
        if (currentToken.type == TokenType::KeyWord){
            logat("Next statement is keyword","Parser.pS");
            if (currentToken.value == "if"){
                state = parseIf();
            } else if (currentToken.value == "tag"){
                state = parseTag();
            } else if (currentToken.value == "drop"){
                state = parseDrop();
            } else if (currentToken.value == "class"){
                state = parseClass();
            } else if (currentToken.value == "match"){
                state = parseMatch();
            } else if (currentToken.value == "mod"){
                state = parseMod();
            } else if (currentToken.value == "todo"){
                state = parseTodo();
            } else if (currentToken.value == "assert"){
                state = parseAssert();
            } else if (currentToken.value == "fn"){
                state = parseFunction();
            } else if (currentToken.value == "struct"){
                state = parseStruct();
            } else if (currentToken.value == "enum"){
                return parseEnum();
            } else if (currentToken.value == "break"){
                state = parseBreak();
            } else if (currentToken.value == "loop"){
                state = ParseLoop();
            } else if (currentToken.value == "while"){
                state = ParseWhile();
            } else if (currentToken.value == "return"){
                state = ParseRet();
            } else if (currentToken.value == "extern"){
                state = parseExtern();
            } else if (currentToken.value == "pub"){
                state = parsePub();
            } else if (currentToken.value == "inline"){
                state = parseInline();
            } else if (currentToken.value == "import"){
                state = parseImport();
            } else if (currentToken.value == "convention"){
                state = parseConvention();
            } else if (currentToken.value == "mut"){
                advance(); //mut
                auto lline = lexer.line;
                auto mutpos = lexer.poswline;
                std::shared_ptr<ASTNode> masn = parseStatement();

                if (auto anode = std::dynamic_pointer_cast<AssignNode>(masn)){
                    //println(anode->getVarName());
                    anode->setMut(true);
                    state = anode;

                } else if (auto sanode = std::dynamic_pointer_cast<StrongAssignNode>(masn)){
                    sanode->setMut(true);
                    state = sanode;
                } else {
                    displayError("mut/var keyword is not supported for this type",lline,mutpos,lexer.poswline,gerr({"reason","the 'mut'/'var' keyword can only be used for where something is being assigned, this occurs in assigning directly, defining a function, etc","note","the mut keyword and var keyword are aliases for eachother","[X]","a = mut 7","[O]","var a = 7"}) );
                    state = masn;
                }

            }

        }
        auto bpos = lexer.prevposwline;

        if (state){
            if (aline == lexer.line){
                std::vector<int> inp{aline,apos,bpos};
                lpos[state->id] = inp;
            } else {
                if (lexer.linemaxpos.find(aline) != lexer.linemaxpos.end()){
                    std::vector<int> inp{aline,apos,lexer.linemaxpos[aline]};
                    lpos[state->id] = inp;
                }
            }
        }

        if (state){
            if (currentToken.type == TokenType::Semicolon){

                advance();
            }
            return state;
        }



        return parseExpression();
    }
    std::string consume(TokenType T){
        std::string v = currentToken.value;
        match(T);
        return v;
    }
    std::shared_ptr<AssignNode> parseStruct(){
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> keys;
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> cons;
        advance(); // struct
        std::string vn = consume(TokenType::Identifier);
        match(TokenType::LBrace);
        while (currentToken.type != TokenType::RBrace){
            std::string typer = consume(TokenType::Identifier);
            if (currentToken.type == TokenType::Walrus){
                advance();
                cons[typer] = parseExpression();
            } else {
            consume(TokenType::Colon);
            keys[typer] = parseExpression();
            }
            if (currentToken.type != TokenType::RBrace){
                consume(TokenType::COMMA);
            }

        }
        advance(); // }
        structs[vn] = 0;
        return std::make_shared<AssignNode>(vn,std::make_shared<StructDeclNode>(keys,cons));
    }


    std::shared_ptr<BreakNode> parseBreak(){
        int l = lexer.line;
        advance(); // break
        std::shared_ptr<ASTNode> expr;
        if (lexer.line == l){
            
            expr = parseExpression();
        }
        return std::make_shared<BreakNode>(expr);
    }

    std::shared_ptr<WhileNode> ParseWhile(){
        advance(); // while
        auto expr = parseExpression();
        auto body = parseBlock();
        return std::make_shared<WhileNode>(std::make_shared<ExpressionNode>(expr,body));
    }
    std::shared_ptr<ASTNode> parseIfLet(){
        advance(); // let
        auto assignexpr = parseExpression();
        auto body = parseExpression();
        if (currentToken.value != "else"){
            throw std::runtime_error("Else is require for iflet");
        }
        match(TokenType::KeyWord);
        auto elser = parseExpression();
        return std::make_shared<IfLetNode>(assignexpr,body,elser);
    }
    // Parse any expression with precedence handling
    std::shared_ptr<ASTNode> parseIf(){
        advance(); // if
        if (currentToken.value == "let"){
            
            return parseIfLet();
        }
        std::shared_ptr<ASTNode> main = std::dynamic_pointer_cast<ASTNode>(parseExpression()); // 1 == 1
        std::shared_ptr<ASTNode> mainbody = std::dynamic_pointer_cast<ASTNode>(parseExpression());; // {}
        std::shared_ptr<ExpressionNode> ifmain = std::make_shared<ExpressionNode>(main,mainbody);
        tsl::ordered_map<int, std::shared_ptr<ExpressionNode>> elifs;
        int a = 0;

        while (currentToken.value == "elif") {
            advance(); // elif
            std::shared_ptr<ASTNode> expr = std::dynamic_pointer_cast<ASTNode>(parseExpression());
            std::shared_ptr<ASTNode> body = std::dynamic_pointer_cast<ASTNode>(parseExpression());
            elifs[a] = std::make_shared<ExpressionNode>(expr, body);
            a += 1;
        }

        std::shared_ptr<ExpressionNode> elsemain;
        if (currentToken.value == "else") {
            advance(); // else

            std::shared_ptr<ASTNode> elsebody = std::dynamic_pointer_cast<ASTNode>(parseExpression());
            elsemain = std::make_shared<ExpressionNode>(nullptr,elsebody);
        }

        return std::make_shared<IFNode>(ifmain, elifs, elsemain);


    }

    std::shared_ptr<ASTNode> ParseIndexSlice(std::shared_ptr<ASTNode> lhs){
        int posi = lexer.getCpos();
        advance(); // [
        auto v = parseExpression(); // 1 / 1+1 / adapt for slice later
        if (currentToken.type == TokenType::Colon){
            rb(posi);
            return lhs;
        }
        if (currentToken.type == TokenType::COMMA){
            rb(posi);
            return lhs;
        }
        match(TokenType::RBracket); // ]
        return std::make_shared<IndexNode>(lhs,v);
    }
    std::shared_ptr<ASTNode> ParseLoop(){
        advance(); // loop
        std::shared_ptr<ASTNode> times;
        if (currentToken.type == TokenType::Integer){
            //println(currentToken.value);
            times = parseIntLiteral();
            //println(convertToString(std::dynamic_pointer_cast<IntLiteralNode>(times)->getValue()));
        } else {
            times = std::make_shared<ASTNode>();
        }
        auto v = parseBlock(); // {}
        
        return std::make_shared<LoopNode>(v,times);
    }
    std::shared_ptr<ASTNode> parsemember(std::shared_ptr<ASTNode> lhs){
        if (auto ltest = std::dynamic_pointer_cast<IntLiteralNode>(lhs)){
        advance();
        

        auto rhs = parseIntLiteral();
        std::string l_str = std::to_string(std::dynamic_pointer_cast<IntLiteralNode>(lhs)->getValue());
        std::string r_str = std::to_string(std::dynamic_pointer_cast<IntLiteralNode>(rhs)->getValue());


        bool isNegative = (l_str[0] == '-');
        

        if (isNegative) {
            l_str = l_str.substr(1);
        }


        std::string combined = l_str + "." + r_str;


        double result = std::stod(combined);

        

        return std::make_shared<ONode>(Object(result, "f64", isNegative));
        }

        std::shared_ptr<ASTNode> mem = lhs;
        if (auto lh = std::dynamic_pointer_cast<IdentifierNode>(lhs)){
            mem->setPointsTO(lh->getValue());
        }

        
        
        
        if (currentToken.type == TokenType::DOT){
            while (currentToken.type == TokenType::DOT){
                advance(); //.
                std::string p = currentToken.value; // access pt eg lhs.rhs.rec
                advance();
                if (currentToken.type == TokenType::Assign){
                    advance();
                    mem = std::make_shared<MemAccNode>(mem, std::make_shared<StringLiteralNode>(p),true,parseExpression());
                } else {
                    mem = std::make_shared<MemAccNode>(mem, std::make_shared<StringLiteralNode>(p));
                }

            }

        }
        
        return mem;
        
    }
    std::shared_ptr<ASTNode> parseCall(std::shared_ptr<ASTNode> lhs){
        std::string ptsto;
        if (auto id = std::dynamic_pointer_cast<IdentifierNode>(lhs)){
            ptsto = id->getValue();
        }
        tsl::ordered_map<int,std::shared_ptr<ASTNode>> dat;
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> inserts;
        match(TokenType::LParen); // (
        int a = 0;
        while (currentToken.type != TokenType::RParen){
            auto expr = parseExpression();
            if (currentToken.type == TokenType::Colon){
                if (auto id = std::dynamic_pointer_cast<IdentifierNode>(expr)){
                    advance(); // :
                    inserts[id->getValue()] = parseExpression();
                }
            } else {
                dat[a] = expr;
            }
            a += 1;
            if (currentToken.type != TokenType::RParen){
                match(TokenType::COMMA);
            }
        }
        match(TokenType::RParen); // )
        auto retnode = std::make_shared<CallNode>(dat,lhs,inserts);
        //retnode->setPointsTO(ptsto);
        return retnode;
    }

        

    std::shared_ptr<ASTNode> ParseUnarary(std::shared_ptr<ASTNode> lhs){
        match(TokenType::Unarary);
        auto iftrue = parseExpression();
        auto BN = std::make_shared<BlockNode>();
        BN->addStatement(iftrue);
        std::shared_ptr<ExpressionNode> ifmain = std::make_shared<ExpressionNode>(lhs,BN);
        match(TokenType::Bang);
        auto elser = parseExpression();
        auto BN2 = std::make_shared<BlockNode>();
        BN2->addStatement(elser);
        auto elsemain = std::make_shared<ExpressionNode>(std::make_shared<ASTNode>(),BN2);
        tsl::ordered_map<int, std::shared_ptr<ExpressionNode>> elses = {};
        auto ifn = std::make_shared<IFNode>(ifmain,elses,elsemain);
        isused.push_back(ifn->id);
        return ifn;
        // IFNode(const std::shared_ptr<ExpressionNode>& value,const tsl::ordered_map<int,std::shared_ptr<ExpressionNode>> elses,const std::shared_ptr<ExpressionNode>& nott)

    }
    std::shared_ptr<ASTNode> ParseNUnarary(std::shared_ptr<ASTNode> lhs){
        match(TokenType::NUnarary);
        
        auto elser = parseExpression();
        return std::make_shared<NUnararyNode>(lhs,elser);

    }
    std::shared_ptr<ASTNode> parseassign(std::shared_ptr<ASTNode> lhs){
        advance(); // =
        return std::make_shared<ExprAssignNode>(lhs,parseExpression());
    }
    bool lookAheadForIrreg(std::shared_ptr<ASTNode> lhs,tsl::ordered_map<std::string,int> structs){
        if (auto SD = std::dynamic_pointer_cast<StructDeclNode>(lhs) ){
            return true;
        } else if (auto ID = std::dynamic_pointer_cast<IdentifierNode>(lhs)){
            if (structs.find(ID->getValue()) != structs.end()){
                return true;
            }
        }
        return false;
    }
    std::shared_ptr<ASTNode> ParseDecon(std::shared_ptr<ASTNode> lhs = NULL){
            advance(); // |
            std::vector<std::string> idents;
            while (currentToken.type != TokenType::Deconstruct){
                idents.push_back(consume(TokenType::Identifier));
                if (currentToken.type != TokenType::Deconstruct){
                    match(TokenType::COMMA);
                }
            }
            match(TokenType::Deconstruct);
            return std::make_shared<DeconsNode>(lhs,idents);

    }
    std::shared_ptr<ASTNode> ParseCast(std::shared_ptr<ASTNode> lhs){
        advance(); // as
        auto castto = parseExpression();
        return std::make_shared<CastNode>(lhs,castto);
    }
    std::shared_ptr<ASTNode> parseExpression(int precedence = 0,std::shared_ptr<ASTNode> glhs=NULL,bool cb = false) {
        // Parse the left-hand side (LHS) of the expression first
        if (currentToken.type == TokenType::Semicolon){
            //advance();
            //println("skipped semicolon");
        }
        auto apos = lexer.prevposwline;
        auto l1 = lexer.line;
        std::shared_ptr<ASTNode> lhs;
        if (glhs != NULL){
            logat("GLHS: !NULL","Parser.pE");
            lhs = glhs;
        } else {
            logat("Parsing primary for " + currentToken.value,"Parser.pE");
            lhs = parsePrimary();
        }
        auto l2 = lexer.line;
        auto bpos = lexer.prevposwline;
        if (auto macrotest = std::dynamic_pointer_cast<MacroNode>(lhs)){

        } else {
        if (!cb){
            if (l1 == l2){
                std::vector<int> tl{lexer.line,apos,bpos};
                lpos[lhs->id] = tl;
            } else {

            std::vector<int> tl{l1,bpos,lexer.linemaxpos[l1]};
            lpos[lhs->id] = tl;
            }
            }
        }

        auto clhs = lhs;
        //println("CUR: " + currentToken.value);
        //std::cout<<"CUR: " + currentToken.value;
        //a = lexer.poswline;
        auto bline = lexer.line;
        if (l1 != l2){
            return lhs;
        }
        
        logat("Parsing expression for " + currentToken.value,"Parser.pE");
        if (currentToken.type == TokenType::Semicolon){

            advance();
            return lhs;
        }
        if (currentToken.type == TokenType::KeyWord){
            if (currentToken.value == "as"){
                return ParseCast(lhs);
            }
        }
        if (currentToken.type == TokenType::LBrace && lookAheadForIrreg(lhs,structs) == true){

            lhs = parseStructInstance(lhs);
        }
        
        if (currentToken.value == "<" && lexer.lookAheadForTemp() == true){
            
            //std::cout<< "template looking";
            //lhs = parseTemplateO(lhs);
        }
        if (currentToken.type == TokenType::Deconstruct){
            lhs = ParseDecon(lhs);
        }
        if (currentToken.type == TokenType::NUnarary){
            lhs = ParseNUnarary(lhs);
        }



        if (currentToken.type== TokenType::AmpersandEq){
            lhs = parsePointerEq(lhs);
        } 


        if (currentToken.type == TokenType::LBracket){
            lhs = ParseIndexSlice(lhs);
            if (currentToken.type == TokenType::Assign){
                lhs = parseassign(lhs);
            }
        }
        if (currentToken.type == TokenType::LParen){
            
            lhs = parseCall(lhs);
        }
        if (currentToken.type == TokenType::DOT){
            lhs = parsemember(lhs);
        }        

        //println(tokenTypeToString(currentToken.type));
        while (((currentToken.type == TokenType::OP) && (getPrecedence(currentToken) >= precedence))) {
            Token op = currentToken;

            advance();  

            if (currentToken.type == TokenType::Assign){
                advance();
                std::string accop = op.value;
                
                Token Taop = Token{value: accop};
                int nextPrecedence = getPrecedence(Taop) + (isRightAssociative(Taop) ? 0 : 1);
                auto rhs = parseExpression();
                std::shared_ptr<IdentifierNode> id = std::dynamic_pointer_cast<IdentifierNode>(lhs);
                return std::make_shared<AssignNode>(id->getValue(),std::make_shared<BinOP>(id,rhs,accop));
            }

            int nextPrecedence = getPrecedence(op) + (isRightAssociative(op) ? 0 : 1);
            auto rhs = parseExpression(nextPrecedence);  
            
            //println("**");
            //println(op.value);
            

            lhs = std::make_shared<BinOP>(lhs, rhs, op.value); 
        }
        if (currentToken.type == TokenType::Unarary){
            lhs = ParseUnarary(lhs);
        }
        int cpos = lexer.prevposwline;
        if (clhs != lhs){
            logat("Fail clhs check","parser.pE");
            lhs = parseExpression(0,lhs,false);
            if (bline == lexer.line){
                lpos[lhs->id] = {bline,bpos,cpos};
            } else {
                lpos[lhs->id] = {bline,bpos,lexer.linemaxpos[bline]};
            }

        } 
        

        
        return lhs;
    }
    
    std::shared_ptr<ASTNode> parsePointer(){
        bool isborrow=false;
        
        if (currentToken.type == TokenType::AmpersandB){
            isborrow=true;
        }
        advance(); // &/&b
        if (currentToken.type == TokenType::KeyWord && currentToken.value == "mut"){
            advance();
            return std::make_shared<PointerNode>(parseExpression(),true,isborrow);
        }
        return std::make_shared<PointerNode>(parseExpression(),false,isborrow);
    }
    std::shared_ptr<ASTNode> parseDeref(){
        advance(); // *
        return std::make_shared<DerefNode>(parseExpression());
    }
    std::shared_ptr<ASTNode> parseOwner(){
        advance(); // &g
        return std::make_shared<GiveOwnershipNode>(parseExpression());
    }
    std::shared_ptr<ASTNode> parsePointerEq(std::shared_ptr<ASTNode> lhs){
        advance(); // &=
        return std::make_shared<ModifyPtrNode>(lhs,parseExpression());
    }

    std::shared_ptr<ASTNode> parseSearchable(){
        advance(); // [|
        std::string type;
        int a = 0;
        int index = 0;
        tsl::ordered_map<int,std::shared_ptr<ASTNode>> checkers;
        while (currentToken.type != TokenType::RSearchable){
            if (currentToken.type == TokenType::DoubleDot && a == 0){
                advance();
                type =  "lend";
            }
            else if (currentToken.type == TokenType::DoubleDot && a != 0){
                if (type == "rend"){
                    throw std::runtime_error("Middle declerations are currently not supported");
                }
                advance();
                type = "rend";
            } else {
                checkers[index] = parseExpression();

                index += 1;
            }
            if (currentToken.type != TokenType::RSearchable){
                match(TokenType::COMMA);
            }
            a += 1;
        }
        match(TokenType::RSearchable);
        return std::make_shared<SearchableNode>(type,checkers);
    }
    std::shared_ptr<ASTNode> parseSignature(){
        advance(); // sig/signature

        match(TokenType::LParen); // (
        tsl::ordered_map<int, std::string> tick;
        tsl::ordered_map<std::string, std::string> internals;
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> args;
        int id = 0;
        while (currentToken.type != TokenType::RParen){
            args[std::string("a") + std::to_string(id)] = parseExpression();
            if (currentToken.type != TokenType::RParen){
                advance(); // ,
            }
            tick[id] =std::string("a") + std::to_string(id);
            id += 1;
        }
        std::shared_ptr<ASTNode> retty;
        match(TokenType::RParen); // )
        if (currentToken.type == TokenType::ReturnTy){
            advance(); // ->
            retty = parseExpression();
        } else {
            retty = std::make_shared<IdentifierNode>("none");
        }
        args["-!retty"] = retty;
        auto BN = std::make_shared<BlockNode>();
        BN->addStatement(std::make_shared<RetNode>(retty));
        auto node = std::make_shared<MappedFunctionNode>(args,BN,tick,internals);
        issig.push_back(node->id);
        return node;

    }
    std::shared_ptr<ASTNode> parseInternal(){
        advance(); // i/internal
        auto id = consume(TokenType::Identifier);
        auto pub = consume(TokenType::Identifier);
        return std::make_shared<InternalsNode>(id,pub);


    }
    std::shared_ptr<ASTNode> parseUnsafe(){

        advance(); // unsafe/usafe
        int apos = lexer.poswline;
        int aline = lexer.line; 

        auto stat = parseStatement();
        if (auto AN = std::dynamic_pointer_cast<AssignNode>(stat)){
            
            if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(AN->getValue())){
            isunsafedecl[MFN->getBody()->id] = true;
            } else {
                displayError("Unsupported unsafe decree with the below statement",aline,apos,apos+7,gerr({"note","Currently unsafe decrees are only supported for functions"}));
            }
        } 
        else if (auto Pub = std::dynamic_pointer_cast<PubNode>(stat)){
            if (auto AAN = std::dynamic_pointer_cast<AssignNode>(Pub->getValue())){
            if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(AAN->getValue())){
            isunsafedecl[MFN->getBody()->id] = true;
            } else {
                displayError("Unsupported unsafe decree with the below statement",aline,apos,apos+7,gerr({"note","Currently unsafe decrees are only supported for functions"}));
            }
            } else {
                displayError("Unsupported unsafe decree with the below statement",aline,apos,apos+7,gerr({"note","Currently unsafe decrees are only supported for functions"}));
            }
        }
        else {
            displayError("Unsupported unsafe decree with the below statement",aline,apos,apos+7,gerr({"note","Currently unsafe decrees are only supported for functions"}));
        }
        
        return stat;
    }
    std::shared_ptr<ASTNode> parseSafe(){

        advance(); // safe
        int apos = lexer.poswline;
        int aline = lexer.line; 

        auto stat = parseStatement();
        if (auto AN = std::dynamic_pointer_cast<AssignNode>(stat)){

            if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(AN->getValue())){
            issafedecl[MFN->getBody()->id] = true;
            } else {
                displayError("Unsupported safe decree with the below statement",aline,apos,apos+6,gerr({"note","Currently safe decrees are only supported for functions"}));
            }
        }
        else if (auto Pub = std::dynamic_pointer_cast<PubNode>(stat)){
            if (auto AAN = std::dynamic_pointer_cast<AssignNode>(Pub->getValue())){
            if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(AAN->getValue())){
            issafedecl[MFN->getBody()->id] = true;
            } else {
                displayError("Unsupported safe decree with the below statement",aline,apos,apos+7,gerr({"note","Currently safe decrees are only supported for functions"}));
            }
            } else {
                displayError("Unsupported safe decree with the below statement",aline,apos,apos+7,gerr({"note","Currently safe decrees are only supported for functions"}));
            }
            }
        else {
            displayError("Unsupported safe decree with the below statement",aline,apos,apos+6,gerr({"note","Currently safe decrees are only supported for functions"}));
        }
        
        return stat;
    }
    std::shared_ptr<ASTNode> ParseAnon(){
        int ipos = lexer.getCpos();
        return parseFunction();
    }

    std::shared_ptr<ASTNode> ParseFuture(){
        advance(); // future
        auto id = consume(TokenType::Identifier);
        return std::make_shared<FutureNode>(id);
    }

    std::shared_ptr<ASTNode> parseComment(){
        auto cline = lexer.line;
        while (lexer.line == cline){
            advance();
        }
        auto austin = std::make_shared<ASTNode>(); isbadnode.push_back(austin->id); return austin;
    }
    std::shared_ptr<ASTNode> ParseRef(){
        auto apos = lexer.poswline;
        auto aline = lexer.line;
        advance(); // ref
        if (!in("c_refs",allowances)){
        displayError("Unsafe use of C references",aline,apos-4,apos,gerr({"note","You may suppress this warning with the decleration: '#allow(c_refs)'"}),true);
        }
        return std::make_shared<RefNode>(parseExpression());
    }

    std::shared_ptr<ASTNode> parsePrimary() {
        logat("Parsing Primary for: " + tokenTypeToString(currentToken.type) + " " + currentToken.value,"Parser.pP");
        switch (currentToken.type) {
            case TokenType::Decorator:
                return parseDecorator();
            case TokenType::LSearchable:
                return parseSearchable();
            case TokenType::Ampersand:
                return parsePointer();
            case TokenType::AmpersandB:
                return parsePointer();
            case TokenType::AmpersandG:
                return parseOwner();
                
            case TokenType::Integer:
                return parseIntLiteral();
            case TokenType::String:
                return parseStringLiteral();
            case TokenType::Identifier:
                return parseIdentifier();
            case TokenType::LBrace:
                return parseBlock();
            case TokenType::LParen:
                return parseTuple();
            case TokenType::LBracket:
                return parseList();
            case TokenType::OP:
                if (currentToken.value == "-"){
                    advance();
                    std::shared_ptr<ASTNode> nint = parseIntLiteral();

                    if (auto rnint = std::dynamic_pointer_cast<IntLiteralNode>(nint)){
                    rnint->setValue(-1*(rnint->getValue()));
                    return rnint;
                    } else if (auto onint = std::dynamic_pointer_cast<ONode>(nint)){
                        onint->setNVE();
                        return onint;
                    }
                } else if (currentToken.value == "*"){
                    
                    return parseDeref();
                } else {
                    displayError("Unexpected token: '" +  currentToken.value + "'" ,lexer.line,0,lexer.poswline,{});
                    return nullptr;
                }
            case TokenType::KeyWord: // assignable keywords
            if (currentToken.value == "if"){
                auto IFN = parseIf();
                isused.push_back(IFN->id);
                return IFN;
            } else if (currentToken.value == "loop"){
                return ParseLoop();
            } else if (currentToken.value == "macro"){
                if (activeMacro){
                    throw std::runtime_error("Cannot define a macro within a macro");
                }
                auto apos = lexer.prevposwline;
                auto aline = lexer.line;
                advance(); // macro
                auto ident = consume(TokenType::Identifier);
                consume(TokenType::LParen);
                consume(TokenType::RParen);
                
                logat("Began Macro","Parser.pP");
                macros[ident] = serial_macro();
                auto bpos = lexer.prevposwline;
                logat("Ended Macro","Parser.pP");
                auto dummynode = std::make_shared<MacroNode>();
                if (aline == lexer.line){
                    lpos[dummynode->id] = {aline,apos,bpos};
                } else {
                    lpos[dummynode->id] = {aline,apos,lexer.linemaxpos[aline]};
                }
                macros[ident]->setNID(dummynode->id);
                return dummynode;
            } else if (currentToken.value == "future"){
                return ParseFuture();
            } else if (currentToken.value == "while"){
                auto WN = ParseWhile();
                isused.push_back(WN->id);
                return WN;
            } else if (currentToken.value == "ref"){
                auto ref = ParseRef();

                return ref;
            } else if (currentToken.value == "match"){
                return parseMatch();
            } else {
                displayError("Keyword '" + currentToken.value + "' cannot be used in an expression.",lexer.line,lexer.poswline,lexer.poswline +currentToken.value.size(),gerr({"reason","Certain keywords can *only* be used by themselves","[X]","func = fn foo(...){} is not allowed","[O]","fn foo(...){} is allowed"}) );
            } 
            case TokenType::Hashtag:
                return parseHashTag();
            case TokenType::Unarary:
                advance();
                return std::make_shared<IdentifierNode>("?");
            case TokenType::Anon:
                return ParseAnon();
            case TokenType::Comment:
                return parseComment();
            case TokenType::LExpr:
                return parseContainedExpr();
            case TokenType::xadecimal:
                return parseHex();

                

                //return std::make_shared<StructDeclNode>(std::vector<std::string>({"chain",";"}));
            default:
            
                //std::cout << "TokenType: " << tokenTypeToString(currentToken.type) << "\n";
                //advance();
                displayError("Unexpected token: '" +  currentToken.value + "'" ,lexer.line,0,lexer.poswline,{});

        }
        return nullptr;
    }
    std::shared_ptr<ASTNode> parseHex(){
        auto hexv = consume(TokenType::xadecimal);
        return std::make_shared<ONode>(Object(std::stoull(hexv),"u64"));
    }
    std::shared_ptr<ASTNode> parseContainedExpr(){
        match(TokenType::LExpr); // #|
        
        auto exp = parseExpression();
        match(TokenType::LExpr);
        return exp;
    }

    std::shared_ptr<ASTNode> parseList(){

        advance(); // [
        tsl::ordered_map<int,std::shared_ptr<ASTNode>> list;
        int a = 0;
        while (currentToken.type != TokenType::RBracket){
            std::shared_ptr<ASTNode> exp = parseExpression(); // 1
            list[a] = exp;
            if (a == 0 && currentToken.type == TokenType::Colon){

                match(TokenType::Colon);
                auto toret = std::make_shared<TSLInitNode>(exp,parseExpression());
                match(TokenType::RBracket);

                return toret;

                // make type safe init node....
            }
            if (currentToken.type != TokenType::RBracket){
                match(TokenType::COMMA); // ,
            a += 1;
            }

        }
        advance(); // ]
        if (ruleset["typing"] == "static"){
        if (list.find(0) == list.end()){
            throw std::runtime_error("empty TSLs can be declared with [<type>:?] or [<type>:0]");
        }
        return std::make_shared<TypeSafeListNode>(list,list[0]);
        }
        return std::make_shared<ListNode>(list);
    }

    // Parse an integer literal
    std::shared_ptr<ASTNode> parseIntLiteral() {
        std::string ival = currentToken.value;
        match(TokenType::Integer);
        
        if (currentToken.type == TokenType::Identifier && converters.find(currentToken.value) != converters.end()){
            int64_t rval = safeStringToInt64(ival);
            std::string type = currentToken.value;
            match(TokenType::Identifier);
            return std::make_shared<ONode>(Object(rval,type));
        }
        std::shared_ptr<ASTNode> intNode;
        logat("String to int","pIL");
        try {
        
            intNode = std::make_shared<IntLiteralNode>(std::stoi(ival));
        } catch (std::out_of_range& e) {
            if (!AP.has("-noautomatives")){
                intNode = std::make_shared<ONode>(Object(safeStringToInt64(ival),"i64"));
            }
        }
        return intNode;
    }

    // Parse a string literal
    std::shared_ptr<ASTNode> parseStringLiteral() {
        auto stringNode = std::make_shared<StringLiteralNode>(currentToken.value);
        advance();
        return stringNode;
    }


    // Parse an identifier
    std::shared_ptr<ASTNode> parseIdentifier() {
        logat("Parsing Identifier","Parser.pI");
        auto varName = currentToken.value;
        advance();
        if (macros.find(varName) != macros.end()){
            auto mB = std::dynamic_pointer_cast<MacroBlock>(macros[varName]);
            invokingMacro = true;
            macroinfo["errorpos"] = lpos[mB->NID];
            macroinfo["content"] = std::string("During invokation of the above macro, the above error occurred");
            macroinfo["hint"] = std::string("Failed to invoke macro");
            auto ret = invoke_macro(mB);
            invokingMacro = false;
            macroinfo.clear();
            if (!ret){
                displayError("During invokation of macro; nothing was returned",lpos[mB->NID][0],lpos[mB->NID][1],lpos[mB->NID][2],gerr({"note", "It is recommended to ensure the macro expands to a statement"}),true);
                return std::make_shared<ASTNode>();
            }
            return ret;
        }
        if (macroVars.find(varName) != macroVars.end() && activeMacro){
            return std::make_shared<DoubleRefNode>(macroVars[varName]);
        }


        
        //println("IDENT");
        //println(currentToken.value);
        //println(tokenTypeToString(currentToken.type));

        // Handle variable assignments (e.g., `x = ...`)
        if (currentToken.type == TokenType::Assign && aassign == true) {
            advance();
            //println("ASSIGNING");
            auto value = parseExpression();
            isused.push_back(value->id);
            return std::make_shared<AssignNode>(varName, value);
        } else if (currentToken.type == TokenType::Colon){
            auto posi = lexer.getCpos();
            advance(); // :
            aassign = false;

            auto strongtype = parseExpression();
            
            aassign = true;

            //std::cout<<"untrue?" << strongtype;
            
            //println(strongtype);


            if (currentToken.type != TokenType::Assign){
                rb(posi);
                return std::make_shared<IdentifierNode>(varName);
            }
            match(TokenType::Assign);
            auto value = parseExpression();
            return std::make_shared<StrongAssignNode>(varName,value,strongtype);
        } 

        // If not assignment, it's just a variable reference
        return std::make_shared<IdentifierNode>(varName);
    }

    std::shared_ptr<ASTNode> parseStructInstance(std::shared_ptr<ASTNode> vn){
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> fields;
        //println("STRUCTISM");
        match(TokenType::LBrace);// {
        while (currentToken.type != TokenType::RBrace){
            std::string fn = consume(TokenType::Identifier);
            if (currentToken.type == TokenType::RBrace){
                fields[fn] = std::make_shared<IdentifierNode>(fn);
            }
            else if (currentToken.type == TokenType::COMMA ){
                advance(); // ,
                fields[fn] = std::make_shared<IdentifierNode>(fn);
            } else {

                match(TokenType::Colon);
                fields[fn] = parseExpression();
                if (currentToken.type == TokenType::RBrace){
                    break;
                }
                match(TokenType::COMMA);
            }
        }
        advance(); // }
        //println("Done Struct");
        return std::make_shared<StructInstanceNode>(fields,vn);
    }

    // Parse an expression inside parentheses (i.e., (...))
    std::shared_ptr<ASTNode> parseTuple() {
        advance(); // (
        tsl::ordered_map<int,std::shared_ptr<ASTNode>> tuple;
        int a = 0;
        while (currentToken.type != TokenType::RParen){
            std::shared_ptr<ASTNode> exp = parseExpression(); // 1
            tuple[a] = exp;
            if (currentToken.type != TokenType::RParen){
                match(TokenType::COMMA); // ,
            a += 1;
            }

        }
        advance(); // )

        return std::make_shared<TupleNode>(tuple);
    }

    // Get the precedence of a binary operator
    int getPrecedence(const Token& token) {
        if (token.value == "==" || token.value == "!=") {
            return 1;
        } else if (token.value == ">" || token.value == "<" || token.value == ">=" || token.value == "<=") {
            return 2;
        } else if (token.value == "+" || token.value == "-") {
            return 3;
        } else if (token.value == "*" || token.value == "/" || token.value == "%") {
            return 4;
        }
        return 0;  // Unknown operator has lowest precedence
    }

    bool isRightAssociative(const Token& token) {

        return false;
    }
};



void setscope(std::shared_ptr<Scope> scope);


// type_instructions to the compiler part
std::shared_ptr<type_instruction> intty(std::shared_ptr<Instruction> inst){
    std::vector<std::shared_ptr<Instruction>> insts{inst};
    return std::make_shared<type_instruction>(nullptr,nullptr,insts);
}


std::shared_ptr<type_instruction> nety(std::any v){
    //logat("Nety","nety");
    if (v.type() == typeid(int) || v.type() == typeid(int32_t)){
        return intty(std::make_shared<resource_instruction>("i32",v));
    } else if (v.type() == typeid(std::string)){
        return intty(std::make_shared<resource_instruction>("RawString",v));
    } else if (v.type() == typeid(Object)){
        auto obj = std::any_cast<Object>(v);
        return intty(std::make_shared<resource_instruction>(obj.getType(),obj.GetStore()));
    } else if (v.type() == typeid(char)){
        return intty(std::make_shared<resource_instruction>("Character",v));
    } else if (v.type() == typeid(signed char)){
        return intty(std::make_shared<resource_instruction>("Character",static_cast<char>(std::any_cast<signed char>(v))));
    } else if (v.type() == typeid(int64_t)){
        return intty(std::make_shared<resource_instruction>("i64",v));
    } else if (v.type() == typeid(uint64_t)){
        return intty(std::make_shared<resource_instruction>("u64",v));
    } else if (v.type() == typeid(double)){
        return intty(std::make_shared<resource_instruction>("f64",v));
    } else if (v.type() == typeid(short)){
        return intty(std::make_shared<resource_instruction>("i16",v));
    } else if (v.type() == typeid(unsigned short)){
        return intty(std::make_shared<resource_instruction>("u16",v));
    } else if (v.type() == typeid(unsigned int)){
        return intty(std::make_shared<resource_instruction>("u32",v));
    } else if (v.type() == typeid(unsigned char)){
        return intty(std::make_shared<resource_instruction>("u8",v));
    } else if (v.type() == typeid(bool)){
        return intty(std::make_shared<resource_instruction>("Bool",v));
    } else if (v.type() == typeid(nullificationv)){
        std::vector<std::shared_ptr<Instruction>> elems{intty(std::make_shared<resource_instruction>("Character",static_cast<char>(102)))};
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"pointer");
    } else if (v.type() == typeid(nullification)){
        
        return intty(std::make_shared<resource_instruction>("none",""));
    } else if (v.type() == typeid(nullificationptr)){
        
        return intty(std::make_shared<resource_instruction>("Null",""));
    } else if (v.type() == typeid(Pointer)){
        auto ptr = std::any_cast<Pointer>(v);
        std::vector<std::shared_ptr<Instruction>> elems{nety(ptr.getptr()->getValue())};
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"pointer");
    } else if (v.type() == typeid(BorrowedPointer)){
        auto bptr = std::any_cast<BorrowedPointer>(v);
        std::vector<std::shared_ptr<Instruction>> elems{nety(bptr.getptr()->getValue())};
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"pointer");
    } else if (v.type() == typeid(StructDecl)){
        auto StructD = std::any_cast<StructDecl>(v);
        std::vector<std::shared_ptr<Instruction>> elems;
        for (auto& f : StructD.getValue()){
            elems.push_back(nety(f.second));
        }
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"struct");
    } else if (v.type() == typeid(StructInstance)){
        auto StructD = std::any_cast<StructInstance>(v);
        std::vector<std::shared_ptr<Instruction>> elems;
        for (auto& f : StructD.getValue()){
            elems.push_back(nety(f.second));
        }
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"struct");
    } else if (v.type() == typeid(Tuple)){
        auto Tup = std::any_cast<Tuple>(v);
        std::vector<std::shared_ptr<Instruction>> elems;
        for (auto& f : Tup.getcomparer()){
            elems.push_back(nety(f.second));
        }
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"struct");
    } else if (v.type() == typeid(MappedFunction)){
        auto MFN = std::any_cast<MappedFunction>(v);
        std::vector<std::shared_ptr<Instruction>> elems;
        for (auto& f : MFN.getargs()){
            

            elems.push_back(nety(f.second));
        }
        elems.push_back(nety(MFN.getretty()));
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"fty");
    } else if (v.type() == typeid(ModuleMethod)){
        auto MFN = std::any_cast<MappedFunction>(std::any_cast<ModuleMethod>(v).mfn);
        std::vector<std::shared_ptr<Instruction>> elems;
        for (auto& f : MFN.getargs()){

            elems.push_back(nety(f.second));
        }
        elems.push_back(nety(MFN.getretty()));
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"fty");
    } else if (v.type() == typeid(TagFunction)){
        auto MFN = std::any_cast<MappedFunction>(std::any_cast<TagFunction>(v).MFNlink);
        std::vector<std::shared_ptr<Instruction>> elems;
        for (auto& f : MFN.getargs()){

            elems.push_back(nety(f.second));
        }
        elems.push_back(nety(MFN.getretty()));
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"fty");
    } else if (v.type() == typeid(TypeSafeList)){
        auto tsl = std::any_cast<TypeSafeList>(v);
        std::vector<std::shared_ptr<Instruction>> elems;
        for (int i = 0; i != tsl.size; i++){
            elems.push_back(nety(tsl.get(0)));
        }
        return std::make_shared<type_instruction>(nullptr,nullptr,elems,"array");
    } else {
        println("Failed to get type: "+ demangle(v.type().name()));
        int* idk = nullptr;
        *idk = 10;
        throw std::runtime_error("Unknown type: " + demangle(v.type().name()));
    }

}

#endif