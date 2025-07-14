
#include "base.hpp"

#include "interpreterc.hpp"



#include "Scope.hpp"


#ifndef TOKENS
#define TOKENS
enum class TokenType {
    Identifier,
    Assign,   // '='
    Colon, // :
    Walrus, // :=
    Semicolon, // ;
    Integer,
    String,
    DOT, // .
    OP, // == and etc
    COMMA, // ,
    LBracket, // [
    RBracket, // ]
    LParen, // (
    RParen, // )
    LBrace,   // '{'
    RBrace,   // '}'
    KeyWord,
    Ampersand, // &
    AmpersandB, // &b
    AmpersandG, // &g
    AmpersandEq, // &=
    Bang, // !
    Unarary, // ?
    NUnarary, // ??
    ReturnTy, // ->
    Anon, // ||
    Decorator,
    Hashtag, // #
    DoubleDot, // ..
    LSearchable, // [|
    RSearchable, // |]
    Deconstruct, // %| x, y, |%
    EqArrow, // =>
    Variadic, // ...
    Comment, // //
    LExpr, // |
    RExpr, // |
    xadecimal, // 0x/0b/0o
    EndOfFile,
    Invalid
};

std::unordered_map<std::string, TokenType> TokenTypeMap = {
    {"Identifier", TokenType::Identifier},
    {"Assign", TokenType::Assign},
    {"Colon", TokenType::Colon},
    {"Walrus", TokenType::Walrus},
    {"Semicolon", TokenType::Semicolon},
    {"Integer", TokenType::Integer},
    {"String", TokenType::String},
    {"DOT", TokenType::DOT},
    {"OP", TokenType::OP},
    {"COMMA", TokenType::COMMA},
    {"LBracket", TokenType::LBracket},
    {"RBracket", TokenType::RBracket},
    {"LParen", TokenType::LParen},
    {"RParen", TokenType::RParen},
    {"LBrace", TokenType::LBrace},
    {"RBrace", TokenType::RBrace},
    {"KeyWord", TokenType::KeyWord},
    {"Ampersand", TokenType::Ampersand},
    {"AmpersandB", TokenType::AmpersandB},
    {"AmpersandG", TokenType::AmpersandG},
    {"AmpersandEq", TokenType::AmpersandEq},
    {"Bang", TokenType::Bang},
    {"Unarary", TokenType::Unarary},
    {"NUnarary", TokenType::NUnarary},
    {"ReturnTy", TokenType::ReturnTy},
    {"Anon", TokenType::Anon},
    {"Decorator", TokenType::Decorator},
    {"Hashtag", TokenType::Hashtag},
    {"DoubleDot", TokenType::DoubleDot},
    {"LSearchable", TokenType::LSearchable},
    {"RSearchable", TokenType::RSearchable},
    {"Deconstruct", TokenType::Deconstruct},
    {"EqArrow", TokenType::EqArrow},
    {"Variadic", TokenType::Variadic},
    {"Comment", TokenType::Comment},
    {"LExpr", TokenType::LExpr},
    {"RExpr", TokenType::RExpr},
    {"xadecimal", TokenType::xadecimal},
    {"EndOfFile", TokenType::EndOfFile},
    {"Invalid", TokenType::Invalid}
};

// not strictly neccesary but will be useful for improved error handling later
std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Assign: return "Assign";
        case TokenType::Integer: return "Integer";
        case TokenType::String: return "RawString";
        case TokenType::LBrace: return "LBrace";
        case TokenType::RBrace: return "RBrace";
        case TokenType::OP: return "Op";
        case TokenType::COMMA: return "COMMA";
        case TokenType::LBracket: return "LBracket";
        case TokenType::RBracket: return "RBracket";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::KeyWord: return "KeyWord";
        case TokenType::EndOfFile: return "EndOfFile";
        case TokenType::Invalid: return "Invalid";
        case TokenType::Colon: return "Colon";
        case TokenType::ReturnTy: return "ReturnTy";
        case TokenType::Semicolon: return "SemiColon";
        default: return "Unknown";
    }
}

struct Token {
    TokenType type;
    std::string value;
};

#endif