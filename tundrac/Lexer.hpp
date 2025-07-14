#include "base.hpp"
#include "interpreterc.hpp"
#include "Scope.hpp"
#include "Tokens.hpp"


#ifndef LEXER
#define LEXER

class Lexer {
public:
    Lexer(const std::string& input) : input(input), currentPos(0) {}
    int line = 1;
    tsl::ordered_map<int,int> linemaxpos;
    int poswline = 0;
    int cdiff = 0;
    int PrevPos = 0;
    std::unordered_map<int, std::vector<int>> cposlineranges;
    bool DO_NOT_PROCEED = false;
    int prevposwline = 0;
    bool lookAheadForTemp(){
        //std::cout<<"Looking rn";
        //std::cout<<input.substr(currentPos,5);
        

        
        int c = currentPos;
        std::string id;
        if (input.substr(c,1) == "<"){
            c++;
        }
        while (input.substr(c,1) != ">"){
            if (input.substr(c,1) == ""){
                
                return false;
            }
            if (input.substr(c,1) == "," ){
            if (converters.find(id) != converters.end()){
                return true;
            } else {
                
                
                return false;
            }
                id = "";
            } else {
                id += input.substr(c,1);
            }
            //std::cout <<std::endl<< id<<std::endl;
            
            c++;
        }
        if (converters.find(id) != converters.end()){
            return true;
            } else {
                
                
            return false;
        }
    }
    int getCpos(){
        return currentPos;
    }
    int getPrevpos(){
        return PrevPos;
    }
    void rollback(int setto){
        //throw std::runtime_error("Rollback is depricated; Use peek instead.");
        currentPos = setto;
    }
    Token peek(int times){
        DO_NOT_PROCEED = true;
        Token res;
        int cpos = getPrevpos();
        while (times != 0){
            res = getNextToken();
            logat("Peeked: " + res.value,"lexer.p");
            times--;
        }
        currentPos = cpos;
        DO_NOT_PROCEED = false;
        return res;
    }
    Token getNextToken() {
        // += /= etc
        

        logat("Getting Next Token at pos(" + std::to_string(currentPos)+ ")","Lexer.gNT");
        std::vector<std::string> op{"==", "!=", "<<",">>",">=","<=","&&","||","+","-","/","*",">","<"};
        std::vector<std::string> keywords{"if","elif","else","mut","loop","while","break","struct","enum","fn","return","assert","mod","todo","drop","let","match","future","class","extern","as","pub","in","out","inout","asm","inline","import","from","ref","tag","convention","macro"};
        //println("pwl: " + std::to_string(poswline) + "\ndiff: " + std::to_string((currentPos - cdiff)) + "\ncPOS: " + std::to_string((currentPos)) + " cDIFF: " + std::to_string((cdiff)));
        
        skipWhitespaceAndComments();
        if (!DO_NOT_PROCEED){
            prevposwline = poswline;
            PrevPos = currentPos;
        }


        
        if (currentPos >= input.size()) {
            return { TokenType::EndOfFile, "" };
        }

        char ch = input[currentPos];
        //std::cout << ch;
        if (input.substr(currentPos,3) == "..."){
            ladvance();
            ladvance();
            ladvance();
            return {TokenType::Variadic,"..."};
        }
        if (input.substr(currentPos,1) == "|"){
            ladvance();

            return {TokenType::LExpr,"|"};
        }
        if (input.substr(currentPos,2) == "//"){
            ladvance();
            
            return getNextToken();
        }
        if (input.substr(currentPos,2) == ".."){
            ladvance();
            ladvance();
            return {TokenType::DoubleDot,".."};
        }
        if (ch == '.') {
            ladvance();
            return { TokenType::DOT, "." };
        }
        if (input.substr(currentPos,2) == "::"){
            ladvance();
            ladvance();
            return {TokenType::DOT,"."};
        }
        if (ch == ';') {
            ladvance();
            return { TokenType::Semicolon, ";" };
        }
        if (input.substr(currentPos,2) == "@|"){
            ladvance();
            ladvance();
            return {TokenType::Anon,"@|"};
        }
        if (input.substr(currentPos,2) == "[|"){
            ladvance();
            ladvance();
            return {TokenType::LSearchable,"[|"};
        }
        if (input.substr(currentPos,2) == "|]"){
            ladvance();
            ladvance();
            return {TokenType::RSearchable,"|]"};
        }

        if (input.substr(currentPos,2) == "=>"){
            ladvance();
            ladvance();
            return {TokenType::EqArrow,"=>"};
        }
        if (ch == '@') {
            ladvance();
            return { TokenType::Decorator, "@" };
        }

        if (input.substr(currentPos,2) == "->"){
            ladvance();
            ladvance();
            return {TokenType::ReturnTy,"->"};            
        }

        if (input.substr(currentPos,2) == ":="){
            ladvance();
            ladvance();
            return {TokenType::Walrus,":="};            
        }

        if (input.substr(currentPos,2) == "??"){
            ladvance();
            ladvance();
            return {TokenType::NUnarary,"??"};            
        }


        if (ch == '?') {
            ladvance();
            return { TokenType::Unarary, "?" };
        }


        
        if (input.substr(currentPos,2) == "&b" && std::isalpha(input[currentPos+2]) == false) {
            
            ladvance();
            ladvance();
            //println("AB");
            //println(input.substr(currentPos,1));
            return { TokenType::AmpersandB, "&b" };
        }
        if (input.substr(currentPos,2) == "&g" && std::isalpha(input[currentPos+2]) == false) {
            
            ladvance();
            ladvance();
            return { TokenType::AmpersandG, "&g" };
        }
        if (input.substr(currentPos,2) == "&=") {
            ladvance();
            ladvance();
            return { TokenType::AmpersandEq, "&=" };
        }
        if (in(input.substr(currentPos,2),op) == true || in(input.substr(currentPos,1),op) == true || (input.substr(currentPos,1) == "b" && in(input.substr(currentPos+1,1),{"|","&","^"}))) {
            return {TokenType::OP, parseoperators()};
        }
        if (ch == '&') {
            ladvance();
            return { TokenType::Ampersand, "&" };
        }

        if (input.substr(currentPos,2) == "#\""){
            return { TokenType::String, parseTagStringLiteral() };
        }
        if (ch == '#') {
            ladvance();
            return { TokenType::Hashtag, "#" };
        }
        if (ch == '{') {
            ladvance();
            return { TokenType::LBrace, "{" };
        } else if (ch == '}') {
            ladvance();
            return { TokenType::RBrace, "}" };
        } 
        if (ch == '(') {
            ladvance();
            return { TokenType::LParen, "(" };
        } 
        if (ch == ')'){
            ladvance();
            return {TokenType::RParen, ")"};

        }
        if (ch == '[') {
            //println("LB");
            ladvance();
            return { TokenType::LBracket, "[" };
        } 
        if (ch == ']'){
            ladvance();
            return {TokenType::RBracket, "]"};

        }
        if (ch == ','){
            ladvance();
            return {TokenType::COMMA, ","};

        }
        
        if (ch == '=') {
            ladvance();
            return { TokenType::Assign, "=" };
        }
        if (ch == ':'){
            ladvance();
            return {TokenType::Colon,":"};
        }
        if (std::isalpha(ch) || ch == '_') {
            std::string pi = parseIdentifier();
            if (in(pi,keywords) == true){
                //std::cout << "keyword " << pi;
                return {TokenType::KeyWord, pi};
            } else if (pi == "var"){
                return {TokenType::KeyWord, "mut"};
            } else if (pi == "extend"){
                return {TokenType::KeyWord, "mod"};
            } else if (pi == "and"){
                return {TokenType::OP, "&&"};
            } else if (pi == "or"){
                return {TokenType::OP, "||"};
            } else if (pi == "is"){
                return {TokenType::OP, "=="};
            } // maybe is not in the future
            return { TokenType::Identifier, pi};
        }

        if (ch == '"' || ch == '\'') {
            return { TokenType::String, parseStringLiteral() };
        }
        if (input.substr(currentPos,2) == "0x"){
            ladvance();
            ladvance();
            auto StartPos = poswline;
            auto start = currentPos;
            while (currentPos < input.size() && (input[currentPos] != ' ' && input[currentPos] != '\n' && (isalpha(input[currentPos]) || isdigit(input[currentPos])) == true)) {
            if (!inT<char>(input[currentPos],{'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','a','b','c','d','e','f'})){
                
                displayError("Hexadecimal representations eg. 0o736 may only contain numbers from 0 to 9 and letters A to F",line,StartPos,poswline+ (poswline == StartPos ? 1 : 0),gerr({}));
            }
            ladvance();
            }

            return {TokenType::xadecimal,convertToString(hexStringToNumber(input.substr(start,currentPos-start)))};
        }
        if (input.substr(currentPos,2) == "0b"){
            ladvance();
            ladvance();
            auto StartPos = poswline;
            auto start = currentPos;
            while (currentPos < input.size() && (input[currentPos] != ' ' && input[currentPos] != '\n' && (isalpha(input[currentPos]) || isdigit(input[currentPos])) == true)) {
            //println("Whaddafak: " + std::to_string(input[currentPos] != '0' && input[currentPos] != '1'));
            if (input[currentPos] != '0' && input[currentPos] != '1'){
                //std::cout << "Line: " << line << "Start: " << StartPos << "end: " << poswline << std::endl;
                displayError("Binary representations eg. 0b101 may only contain 0 or 1",line,StartPos,poswline+ (poswline == StartPos ? 1 : 0),gerr({"note","Aside from Octal, a Hexadecimal representation is possible as well."}));
            }
            ladvance();
            }

            return {TokenType::xadecimal,convertToString(binaryStringToNumber(input.substr(start,currentPos-start)))};
        }

        if (input.substr(currentPos,2) == "0o"){
            ladvance();
            ladvance();
            auto StartPos = poswline;
            auto start = currentPos;
            while (currentPos < input.size() && (input[currentPos] != ' ' && input[currentPos] != '\n' && (isalpha(input[currentPos]) || isdigit(input[currentPos])) == true)) {
            if (!inT<char>(input[currentPos],{'0','1','2','3','4','5','6','7'})){
                
                displayError("Octal representations eg. 0o736 may only contain numbers from 0 to 7",line,StartPos,poswline+ (poswline == StartPos ? 1 : 0),gerr({"note","Aside from Binary, Octal and Hexadecimal representations are possible as well."}));
            }
            ladvance();
            }

            return {TokenType::xadecimal,convertToString(octalStringToNumber(input.substr(start,currentPos-start)))};
        }

        
        if (std::isdigit(ch) || ch == '-') {
            return { TokenType::Integer, parseNumberLiteral() };
        }
        if (ch == '!') {
            ladvance();
            return { TokenType::Bang, "!" };
        }
        if (input.substr(currentPos,2) == "%|") {
            ladvance();
            ladvance();
            return { TokenType::Deconstruct, "%|" };
        }
        if (input.substr(currentPos,2) == "|%") {
            ladvance();
            ladvance();
            return { TokenType::Deconstruct, "|%" };
        }


        return { TokenType::Invalid, "" };
    }


private:
    std::string input;
    size_t currentPos;

    void ladvance() {
        if (currentPos < input.size()) {
            char ch = input[currentPos];
            if (input.substr(currentPos,2) == "//"){
                
                while (ch != '\n'){
                    
                    currentPos++;
                    if (!DO_NOT_PROCEED){
                        poswline++;
                    }
                    ch = input[currentPos];
                }
            }
            

            if (ch == '\n') {
                while (ch == '\n'){
                currentPos++;
                if (!DO_NOT_PROCEED){
                    linemaxpos[line] = poswline;

                    line++;

                    prevposwline = 0;
                    
                    poswline = 0;
                    
                }
                ch = input[currentPos];
                }
            } else {
                if (!DO_NOT_PROCEED){
                    poswline++;
                }
                currentPos++;
            }
            
        }
    }


    void skipWhitespaceAndComments() {
    while (currentPos < input.size()) {
        char ch = input[currentPos];

        if (std::isspace(ch)) {
            ladvance();
            continue;
        }

        /*if (input.substr(currentPos, 2) == "//") {
            while (currentPos < input.size() && input[currentPos] != '\n') {
                ladvance();
            }
            // Consume the newline character
            if (currentPos < input.size() && input[currentPos] == '\n') {
                println("consumed comm: " + std::to_string(line));
                ladvance();
            }
            continue;
        }*/

        // Break if neither whitespace nor comment
        break;
    }
}

    std::string parseoperators(){
        // == != > < >> << >= <= && ||
        std::vector<std::string> op{"==", "!=", "<<",">>",">=","<=","&&","||","b|","b&","b^"};
        std::string inp;
        inp = input.substr(currentPos, 2);
        
        for (int i = 0; i < op.size() ; i ++){
        if (inp == op[i]){
            ladvance();
            ladvance();
            return inp;
               //Do Stuff
        }}
        inp = input[currentPos];
        ladvance();
        return inp;


    }
    std::string parseIdentifier() {
        size_t start = currentPos;
        while (currentPos < input.size() && (std::isalnum(input[currentPos]) || input[currentPos] == '_')) {
            ladvance();
        }
        return input.substr(start, currentPos - start);

    }
    std::string encodeUTF8(char32_t codePoint) {
        std::string utf8;
        if (codePoint <= 0x7F) {
            utf8.push_back(static_cast<char>(codePoint));
        } else if (codePoint <= 0x7FF) {
            utf8.push_back(static_cast<char>(0xC0 | ((codePoint >> 6) & 0x1F)));
            utf8.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else if (codePoint <= 0xFFFF) {
            utf8.push_back(static_cast<char>(0xE0 | ((codePoint >> 12) & 0x0F)));
            utf8.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else if (codePoint <= 0x10FFFF) {
            utf8.push_back(static_cast<char>(0xF0 | ((codePoint >> 18) & 0x07)));
            utf8.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            utf8.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        }
        return utf8;
    }
    std::string StringRules(){
        if (input.substr(currentPos,2) == "\\u"){
            ladvance();
            ladvance();
            if (input[currentPos] != '{'){
                throw std::runtime_error("Unicode: \\u{}");
            }
            ladvance();
            size_t start = currentPos;
            while (input[currentPos] != '}'){
                ladvance();
            }
            
            std::string result = input.substr(start, currentPos - start);
            ladvance();

            return encodeUTF8(static_cast<char32_t>(hexStringToNumber("0x" + result)));
        } else if (input.substr(currentPos,2) == "\\n"){
            ladvance();
            ladvance();
            return "\n";
        }  else if (input.substr(currentPos,2) == "\\r"){
            ladvance();
            ladvance();
            return "\r";
        }
        auto retstr = input[currentPos];
        ladvance();
        return std::string(1,retstr);
    }
    std::string parseStringLiteral() {
        char ch;
        ch = input[currentPos];
        size_t start = ++currentPos;
        std::string result;
        while (currentPos < input.size() && input[currentPos] != ch) {
            result += StringRules();
        }
        ladvance(); // Skip the closing quote
        return result;
    }
    std::string parseTagStringLiteral() {
        auto ch = input.substr(currentPos,2);
        ladvance();
        ladvance();
        size_t start = currentPos;
        while (currentPos < input.size() && input.substr(currentPos,2) != "\"#") {
            ladvance();
        }
        std::string result = input.substr(start, currentPos - start);
        ladvance(); // Skip the closing quote
        ladvance(); // Skip the closing quote
        return result;
    }

    std::string parseNumberLiteral() {
        size_t start = currentPos;

        while (currentPos < input.size() && (std::isdigit(input[currentPos]) || input[currentPos] == '-' || input[currentPos] == '_')) {
            ladvance();
        }
        return replace(input.substr(start, currentPos - start),"_","");
    }
};
#endif