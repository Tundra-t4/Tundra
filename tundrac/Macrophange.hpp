#include "base.hpp"
#include "AST.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Macro_Types.hpp"
#include "Tokens.hpp"
#ifndef MACRO
#define MACRO

int MACRO_SESSION_ID = 0;
tsl::ordered_map<std::string,std::shared_ptr<std::shared_ptr<ASTNode>>> CmacroVars;
tsl::ordered_map<int, std::shared_ptr<std::shared_ptr<ASTNode>>> dual_map;

class Macro {
    public:
    int id;
    Macro(Parser* parser): parser(parser), id(MACRO_SESSION_ID++) {}

    void setID(int inp){
        id = inp;
    }



    
    std::shared_ptr<ASTNode> SubParse(std::shared_ptr<MacroInstruction> inst){
        parser->activeMacro = true;
        auto res = parseStatement(inst);
        parser->activeMacro = false;
        parser->clearMacroVars();
        return res;
    }
    std::shared_ptr<MacroInstruction> BeginExpansion(){
        parser->activeMacro = true;

        auto res = Serialize();
        parser->activeMacro = false;

        return res;
    }

    std::shared_ptr<MacroInstruction> Serialize(){
        if (parser->currentToken.value == "switch"){ // have to use values not types ugh
            parser->advance(); // switch


            // now uhhh value so
            tsl::ordered_map<std::string,std::shared_ptr<MacroBlock>> cmps;
            std::shared_ptr<MacroInstruction> value;


            value = parseExpression();
            parser->consume(TokenType::LBrace);
            while (parser->currentToken.type != TokenType::RBrace){
                std::string cmp = "_";
                if (parser->currentToken.value != "_"){
                    cmp = parseString();
                } else {
                    parser->advance();
                }
                parser->consume(TokenType::EqArrow);
                cmps[cmp] = std::dynamic_pointer_cast<MacroBlock>(parseBlock());

                
            }
            parser->consume(TokenType::RBrace); // }
            if (cmps.find("_") == cmps.end()){
                throw std::runtime_error("All switch statements must have a default (_) condition");
            }
            return std::make_shared<MacroSwitch>(value,cmps);
            
            
        } else {
            return parseExpression();
        }
    }



    std::vector<std::string> MVs;
    private:
    Parser* parser;


    std::shared_ptr<ASTNode> parseStatement(std::shared_ptr<MacroInstruction> inst){
        if (auto mB = std::dynamic_pointer_cast<MacroBlock>(inst)){
            std::shared_ptr<ASTNode> stat;
            for (auto item : mB->items){
                stat = parseStatement(item);
            }
            return stat;
            
        }
        else if (auto mS = std::dynamic_pointer_cast<MacroSwitch>(inst)){
            for (auto cmp : mS->cmps){
                if (getMacroAccess(std::dynamic_pointer_cast<MacroAccess>(mS->value)) == cmp.first){
                    return parseStatement(cmp.second);
                }
            }
            return parseStatement(mS->cmps["_"]);
            
        }

        else if (auto mA = std::dynamic_pointer_cast<MacroAssign>(inst)){
            auto parsed = parseStatement(mA->value);
            //parser->addMacroVars(mA->ident,parsed);
            //std::cout << mA->id << std::endl;
            if (dual_map.find(mA->id) != dual_map.end()){
                //std::cout << "switchered " << mA->ident << std::endl;
                //(*dual_map[mA->id]) = *parsed;
                (*dual_map[mA->id]) = parsed;
            }
            
            return std::make_shared<ASTNode>();
        }

        else if (auto mI = std::dynamic_pointer_cast<MacroIdentifier>(inst)){
            
            
            return *CmacroVars[mI->value + std::to_string(id)];
        }

        else if (auto mC = std::dynamic_pointer_cast<MacroCall>(inst)){
            //std::cout << mC->value << std::endl;
            if (mC->value == "next"){
                if (mC->args.size() != 0){
                    throw std::runtime_error("Next has 0 arguments");
                }
                parser->advance();
                return std::make_shared<ASTNode>();
            }
            else if (mC->value == "Error"){
                if (mC->args.size() != 1){
                    throw std::runtime_error("Error has 1 argument");
                }
                macroinfo["content"] = "The following macro error was thrown: " + replace(mC->args[0],"{Macro::CurrentToken}",parser->currentToken.value);
                displayError("During invokation of a macro, an error was thrown by the macro (see hint)",parser->lexer.line,parser->lexer.prevposwline+1,parser->lexer.poswline+1,gerr({"note","This usually means the macro did not receive the token it wanted; A syntax error"}));
            }
            else if (mC->value == "parseExpression"){
                if (mC->args.size() != 0){
                    throw std::runtime_error("parseExpression has 0 arguments");
                }
                return parser->parseExpression();
            }
            else if (mC->value == "parsePrimary"){
                //std::cout << "Parsed primary" << std::endl;
                if (mC->args.size() != 0){
                    throw std::runtime_error("parsePrimary has 0 arguments");
                }
                return parser->parsePrimary();
            }
            else if (mC->value == "parseStatement"){
                if (mC->args.size() != 0){
                    throw std::runtime_error("parseStatement has 0 arguments");
                }
                return parser->parseStatement();
            }
            else if (mC->value == "consume"){
                if (mC->args.size() != 1){
                    throw std::runtime_error("consume has 1 argument");
                }
                if (TokenTypeMap.find(mC->args[0]) == TokenTypeMap.end()){
                    throw std::runtime_error("Invalid Token Type specified");
                }
                parser->consume(TokenTypeMap[mC->args[0]]);
                return std::make_shared<ASTNode>();
            }
            else if (mC->value == "expect"){
                if (mC->args.size() != 1){
                    throw std::runtime_error("expect has 1 argument");
                }
                if (TokenTypeMap.find(mC->args[0]) == TokenTypeMap.end()){
                    throw std::runtime_error("Invalid Token Type specified");
                }
                parser->match(TokenTypeMap[mC->args[0]]);
                return std::make_shared<ASTNode>();
            }
            else if (mC->value == "ensure"){
                if (mC->args.size() != 1){
                    throw std::runtime_error("expect has 1 argument");
                }
                if (TokenTypeMap.find(mC->args[0]) == TokenTypeMap.end()){
                    throw std::runtime_error("Invalid Token Type specified");
                }
                if (parser->currentToken.type != TokenTypeMap[mC->args[0]]){
                    parser->match(TokenTypeMap[mC->args[0]]);
                }
                return std::make_shared<ASTNode>();
            }
            throw std::runtime_error("Unknown MacroCall : " + mC->value);
        } 
        else if (auto mE = std::dynamic_pointer_cast<MacroExpression>(inst)){
            return mE->value;
        }
        else {
            return std::make_shared<ASTNode>();
        }

    }

    std::string getMacroAccess(std::shared_ptr<MacroAccess> mA){
        if (mA->value == "CurrentToken"){
            return parser->currentToken.value;
        }

    }
    
    std::string parseString(){
        return parser->consume(TokenType::String);
    }

    std::shared_ptr<MacroInstruction> parseExpression(){
        auto lhs = parsePrimary();

        if (parser->currentToken.type == TokenType::DOT){

            if (auto mi = std::dynamic_pointer_cast<MacroIdentifier>(lhs)){

                if (mi->value == "Macro"){

                    parser->advance(); // ./::

                    std::string Ident = parser->consume(TokenType::Identifier);
                    if (parser->currentToken.type == TokenType::LParen){
                        parser->advance(); // (
                        std::vector<std::string> args;
                        while (parser->currentToken.type != TokenType::RParen){
                            args.push_back(parseString());

                        }
                        parser->advance(); // )
                        return std::make_shared<MacroCall>(Ident,args);


                    } else {

                        return std::make_shared<MacroAccess>(Ident);
                    }
                }
            }
        } else if (!lhs) {
            return std::make_shared<MacroExpression>(parser->parseStatement());
        } 

        return lhs;

    }

    std::shared_ptr<MacroInstruction> parsePrimary(){
        if (parser->currentToken.type == TokenType::LBrace){
            return parseBlock();
        } else if (parser->currentToken.type == TokenType::Identifier){
            auto ident = parser->currentToken.value;


            if ((ident == "Macro")){
                parser->advance(); // ident

                
                
                return std::make_shared<MacroIdentifier>(ident);
            } else {
                
                

                
                if (parser->peek(1).type == TokenType::Assign){
                    parser->advance(); // ident
                    MVs.push_back(ident);
                    parser->advance();
                    
                    auto mA = std::make_shared<MacroAssign>(ident,Serialize());
                    auto node = std::make_shared<IdentifierNode>("macro_dual_map_error_please_report");
                    auto shared_ref = std::make_shared<std::shared_ptr<ASTNode>>(node);
                    parser->addMacroVars(ident,shared_ref);
                    CmacroVars[ident + std::to_string(id)] = shared_ref;
                    dual_map[mA->id] = shared_ref;
                    return mA;
                }
                return nullptr;
            }

        
        } else {
            return nullptr;
        }

    }

    std::shared_ptr<MacroInstruction> parseBlock(){
        parser->consume(TokenType::LBrace);
        
        std::vector<std::shared_ptr<MacroInstruction>> stats;

        while (parser->currentToken.type != TokenType::RBrace){
            stats.push_back(Serialize());
        }
        parser->consume(TokenType::RBrace);
        return std::make_shared<MacroBlock>(stats);

    }


    

};


#endif