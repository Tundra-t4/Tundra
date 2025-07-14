#ifndef BASE
#define BASE
#include <iostream>
#include <any>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cxxabi.h>
#include <memory>
#include <vector>
#include <sstream>
#include <cctype>
#include <variant>
#include <array>
#include <typeinfo>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <cstddef>
#include <type_traits>
#include <cstring>
#include <functional>
#include <iomanip>
#include <polaroid.h>
#include <map>
#include <sjson.hpp>
#include<ordered_map.h>
#include <typeindex>
#include <Hparser.h>


const std::string red_block = "\033[31m";    // Red blk
const std::string green_block = "\033[32m";  // Green blk
const std::string yellow_block = "\x1b[33m";
const std::string cyan_block = "\033[96m";
const std::string topcorner = "┏";
const std::string bottomcorner = "┗";
const std::string toparc = "╭";
const std::string bottomarc = "└▶";
const std::string arcdown = "│";
const std::string linedown = "┣";
const std::string arrow = "⟶";
const std::string darrow = "⇒";
const std::string reset = "\033[0m";
const std::string fcircle = "●";
const std::string circle = "◯";
std::string result = "";
int warns = 0;
std::vector<int> vidids;
auto debug = true;
tsl::ordered_map<std::string,std::vector<char>> objects;
tsl::ordered_map<int,bool> isunsafedecl;
tsl::ordered_map<int,bool> issafedecl;
tsl::ordered_map<std::string,std::string> ptrnowptsto;
std::vector<int> hasbeenwarned;
auto aassign = true;
bool compile = true;
bool compileflag = true;
auto autosavedebug =true;
bool invokingMacro = false;
tsl::ordered_map<std::string,std::any> macroinfo;
std::string logcon;
tsl::ordered_map<std::string,int> ids;
tsl::ordered_map<std::string,std::vector<std::string>> notes;
tsl::ordered_map<int,std::shared_ptr<type_instruction>> forwards;
tsl::ordered_map<int,std::any> forwardscplex;
std::string mangle_rule = "t4";
tsl::ordered_map<int,std::any> forwardscplex2;
std::unordered_map<std::string,std::string> exports;
std::vector<int> isused;

// Cantor Pairing function
// (x+y) (x+y+1)
// ------------- + y
//        2
uint64_t CPf( uint64_t x,uint64_t y){
    if (x>y){std::swap(x,y);}
    return (((x+y) * (x+y+1))/2) + y;

}

void savelog(){
    if (debug){

    std::ofstream LogFile("Tundra.log");
    LogFile << logcon + "Saved log!";
    LogFile.close();
    }
}

void logat(std::string con, std::string from){
    if (debug){
    logcon += ("@" + from +" -> " + con + "\n");
    if (autosavedebug){
        savelog();
    }
    }
}

// debug/helper functions
std::vector<std::string> split(std::string str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string token;


    while (std::getline(ss, token, delimiter)) {
        result.push_back(token);
    }
    
    return result;
}


std::vector<int> isbadnode;
std::vector<int> issig;

bool in(std::string inp, std::vector<std::string> op){
    for (int i = 0; i < op.size() ; i ++){
    if (inp == op[i]){
        return true;
            //Do Stuff
    }}
    return false;
}
template <typename T>
bool inT(T inp, std::vector<T> op){
    for (int i = 0; i < op.size() ; i ++){
    if (inp == op[i]){
        return true;
            //Do Stuff
    }}
    return false;
}
template <typename T, typename K>
const T* reversefind(tsl::ordered_map<T,K> pool, K reversevalue,bool* found=nullptr){
    for (auto& p : pool){
        if (p.second == reversevalue){
            if (found){
                if (typeid(K) == typeid(std::string)){
                }
                *found = true;
            }
            const T* v = &p.first;
        
            return v;

        }
    }
    return nullptr;
    
}

template <typename T>
std::string debugvectorToString(const std::vector<T>& vec) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i != vec.size() - 1) {
            oss << ", "; 
        }
    }
    return oss.str();
}
// ANSI escape codes for styling
const std::string RED = "\033[31m";
const std::string LIGHT_BLUE = "\033[94m";
const std::string BOLD = "\033[1m";
const std::string RESET = "\033[0m";

bool isdeclunsafe = false;
bool isdeclsafe = false;
std::vector<std::string> HashTagOptions;

 

std::string replace(std::string s, std::string from, std::string to)
{
    if(!from.empty())
        for(std::string::size_type pos = 0; (pos = s.find(from, pos) + 1); pos += to.size())
            s.replace(--pos, from.size(), to);
    return s;
}


uint64_t hexStringToNumber(const std::string& hexString) {
    uint64_t number = 0;
    
    
    std::stringstream ss;

    
    ss << std::hex << hexString;

    
    ss >> number;

    return number;
}

uint64_t binaryStringToNumber(const std::string& binString){
    std::string stream = replace(binString,"0b","");
    uint64_t bin = 0;
    for (int i=0;i!=stream.size();i++){
        bin = (bin*2) + std::stoi(std::string(1,stream[i]));
    }
    return bin;

}

uint64_t octalStringToNumber(const std::string& binString){
    std::string stream = replace(binString,"0o","");
    uint64_t bin = 0;
    for (int i=0;i!=stream.size();i++){
        bin = (bin*8) + std::stoi(std::string(1,stream[i]));
    }
    return bin;

}

int64_t safeStringToInt64(const std::string& str) {
    try {
        size_t pos;
        int64_t value = std::stoll(str, &pos);


        if (pos != str.length()) {
            throw std::invalid_argument("Input contains non-numeric characters");
        }

        return value;
    } catch (const std::invalid_argument& e) {
        //println("FAIL");
        
    } catch (const std::out_of_range& e) {
        //println("FAIL2");
        exit(0);
    }

    return NULL; 
}

std::string demangle(const char* mangledName) {
    int status = -1;
    std::unique_ptr<char, void(*)(void*)> result{
        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
        std::free
    };
    if (status != 0) {
        return "Error demangling name";
    }
    return result.get();
}



std::string removeTrailingZeros(const std::string& input) {

    
    double value = std::stod(input);
    

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << value;
    std::string result = oss.str();
    

    size_t pos = result.find_last_not_of('0');
    if (pos != std::string::npos && result[pos] == '.') {
        result = result.substr(0, pos); 
    } else if (pos != std::string::npos) {
        result = result.substr(0, pos + 1); 
    }
    
    return result;
}

class ArgParser {
    public:
    ArgParser(char** argv,int argc): argv(argv), argc(argc){
    
    }
    bool has(std::string value){
        for (int i = 0; i != argc; i++){
            if (value == std::string(argv[i])){
                return true;
            }
        }
        return false;
    }
    void parse(){
        for (int i = 0; i != argc; i++){
            auto key = std::string(argv[i]);
            if(in(key,sflags)){

                std::string val;
                if (argv[i+1][0] == '\''){
                    while (argv[i+1][std::string(argv[i+1]).size()-1] != '\'' ){
                        val += std::string(argv[i+1]) + " ";
                        i++;

                    }
                    val += std::string(argv[i+1]);
                    val = replace(val,"'","\"");
                } else {
                    val = argv[i+1];
                }
                if (has("-sflags") || has("-flags")){
                    std::cout << key << " : " << val << std::endl;
                }
                if (values.find(key) == values.end()){
                    values[key] = std::vector<std::string>{std::string(val)};
                } else {
                    values[key].push_back(val);
                }

            i++;
            } else {
                if (has("-flags")){
                    std::cout << std::string(argv[i]) << std::endl;
                }
            }
            
        }

    }

    tsl::ordered_map<std::string,std::vector<std::string>> values;
    std::vector<std::string> sflags{"linklib","link","emit","linkpath","output","compilerflags","target","compile","ccflags","typing","toolchain","cc","headerpath","t4path","target-cpu"};
    char** argv;
    int argc;
    private:
};
char** chardummy = new char*[3];
auto AP = ArgParser(chardummy,9);



bool safeGetLine(std::istream& is, std::string& line) {
    line.clear();
    std::istream::sentry se(is, true); 
    std::streambuf* sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        if (c == '\n') return true;
        if (c == '\r') {
            if (sb->sgetc() == '\n') sb->sbumpc(); 
            return true;
        }
        if (c == EOF) {
            return !line.empty();
        }
        line += static_cast<char>(c);
    }
}

// Error helper function
std::string readLine(std::string filepath, int lineNumber,bool isnew=true) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Unable to open file " + filepath);
    }
    
    std::string line;
    for (int i = 1; i <= lineNumber && safeGetLine(file, line); ++i) {
        if (i == lineNumber) {
            file.close();
            return line;
        }
    }
    file.close();
    /*if (isnew){
        return readLine(filepath,lineNumber-1,false);
    }*/
    //throw std::runtime_error("Unable to get line " + std::to_string(lineNumber));
    return "";
}

tsl::ordered_map<std::string, std::string> gerr(std::vector<std::string> g){
    tsl::ordered_map<std::string, std::string> x;
    for (size_t i = 0; i < g.size(); ++i) {
        x[g[i]] = g[i+1];
        ++i;
    }
    return x;
}
// Error displayer function
void displayError(std::string error,int errorLine, int startPos, int endPos,
                  tsl::ordered_map<std::string, std::string> notes,bool warning=false,std::vector<std::string> hint={}) {
        logat(error,"dE");
    try {
        bool ishint = hint.size() != 0;
        bool hashint = invokingMacro; // for now
        if (warning && AP.has("-nowarns")){
            warns += 1;
            return;
        }


        std::string filepath = AP.values["compile"][0];
        std::string errorLineContent = readLine(filepath, errorLine);
        std::string beforeLine = errorLine > 1 ? readLine(filepath, errorLine - 1) : "...";
        std::string afterLine = readLine(filepath, errorLine + 1).empty() ? "..." : readLine(filepath, errorLine + 1);
        std::string idr = " ERROR: ";
        std::string linkbottom = hashint ?( bottomarc + " ") : "";
        std::string linktop = hashint ? (toparc + " ") : "";
        std::string linkblock = hashint ? (arcdown + " ") : (ishint ? "  " :"");
        auto chosenblock = red_block;
        if (warning){
            idr = " WARNING: ";
            
        }
        if (ishint){
            idr = BOLD + " " + hint[0] + " " + RESET ;
            chosenblock = cyan_block;

        }
        

        std::cout << BOLD << (ishint ? arcdown + " " : linktop) << (!warning ? (chosenblock + fcircle + reset) : (yellow_block + circle + reset)) <<" " << RESET << filepath << " [" + std::to_string(errorLine)  + ":" + std::to_string(startPos) + ".." + std::to_string(endPos) + "]\n";
        
        if (ishint){
            std::cout << BOLD << bottomarc << reset << "  " << hint[1] << std::endl;
        }


        if (!beforeLine.empty() && errorLine-1 > 0) {
            std::cout << linkblock << LIGHT_BLUE << errorLine - 1 << RESET << std::string(4-std::to_string(errorLine-1).size(), ' ') + "║     " << beforeLine << "\n";
        }
        std::cout << linkblock << LIGHT_BLUE << errorLine << RESET << std::string(4-std::to_string(errorLine).size(), ' ') + "║     " << errorLineContent << "\n";

        
        if (startPos > endPos){
            endPos = errorLineContent.size();
        }
        std::cout << linkblock << std::string(10 + (startPos == 0 ? 0 : 0), ' ') << std::string(startPos, ' ') << RED << std::string(endPos - startPos, (ishint ? '-': '^')) << RESET << (ishint ? ">": "") << idr << error << "\n";

        if (!afterLine.empty()) {
            std::cout << linkblock << LIGHT_BLUE << errorLine + 1 << RESET << std::string(4-std::to_string(errorLine+1).size(), ' ') + + "║     " << afterLine << "\n";
        }
        if (notes.find("reason") != notes.end()){
            std::cout << linkblock << std::string(4, ' ') + "-  " << BOLD << "reason" << ": " << RESET << notes["reason"] << "\n";
            notes.erase("reason");
        }
        if (notes.find("note") != notes.end()){
            std::cout << linkblock << std::string(4, ' ') + "-  " << BOLD << "note" << ": " << RESET << notes["note"] << "\n";
            notes.erase("note");
        }
        if (notes.find("[X]") != notes.end()){
            std::cout << linkblock << std::string(4, ' ') + "=  " << BOLD << "Example" << ": " << RESET << "\n";
            std::cout << linkblock << std::string(4, ' ') + "=  " << BOLD << "[X]" << ": " << RESET << notes["[X]"] << "\n";
            notes.erase("[X]");
            std::cout << linkblock << std::string(4, ' ') + "=  " << BOLD << "[O]" << ": " << RESET << notes["[O]"] << "\n";
            notes.erase("[O]");
        }

        for (const auto& [key, value] : notes) {
            std::cout << linkblock << std::string(4, ' ') + "~  " << BOLD << key << ": " << RESET << value << "\n";
            
        }


        
        if (invokingMacro && !warning){
            std::cout << linkblock << std::endl;
            auto lposmacro = std::any_cast<std::vector<int>>(macroinfo["errorpos"]);
            invokingMacro = false;
            displayError(std::any_cast<std::string>(macroinfo["content"]) ,lposmacro[0],lposmacro[1],lposmacro[2],{},true,{"hint:",std::any_cast<std::string>(macroinfo["hint"])});
        }
        if (!warning){
            std::cout <<  red_block + fcircle + reset + " Build failed." << std::endl;
            exit(0);
        } else {
            std::cout << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << RED << "Failed to display error: '" << error << "' " << e.what() << RESET << "\n";
        exit(0);
    }
}



bool compareAnyTy(std::any a, std::any b);






tsl::ordered_map<std::string,std::string> ruleset;
std::vector<std::string> allowances;





// Helper function to print a string (and spit it out to Tundra.log)
void println(std::string content) {
    if (content == "my_str2"){
        int* dat = nullptr;
        *dat = 10;
    }
    std::cout << content  << "\n";
    logat(content,"println");
}



// converter functions for interpreter
std::string convertToString(std::any input) {
    try {
        if (input.type() == typeid(std::string)) {
            return std::any_cast<std::string>(input);
        }
        else if (input.type() == typeid(bool)) {
            return (std::any_cast<bool>(input) ? "true": "false");
        }
        else if (input.type() == typeid(const char*)) {
            return std::string(std::any_cast<const char*>(input));
        }
        else if (input.type() == typeid(char)) {
            return std::string(1, std::any_cast<char>(input));
        }
        else if (input.type() == typeid(int8_t)) {
            return std::to_string(std::any_cast<int8_t>(input));
        }
        else if (input.type() == typeid(int32_t)) {
            return std::to_string(std::any_cast<int32_t>(input));
        }
        else if (input.type() == typeid(uint8_t)) {
            return std::to_string(static_cast<int>(std::any_cast<uint8_t>(input)));
        }
        
        else if (input.type() == typeid(uint16_t)) {
            return std::to_string(std::any_cast<uint16_t>(input));
        }
        else if (input.type() == typeid(int)) {
            return std::to_string(std::any_cast<int16_t>(input));
        }
        else if (input.type() == typeid(uint32_t)) {
            return std::to_string(std::any_cast<uint32_t>(input));
        }
        else if (input.type() == typeid(uint64_t)) {
            return std::to_string(std::any_cast<uint64_t>(input));
        }
        else if (input.type() == typeid(int64_t)) {
            return std::to_string(std::any_cast<int64_t>(input));
        }
        else if (input.type() == typeid(double)) {
            return removeTrailingZeros(std::to_string(std::any_cast<double>(input)));
        }    else if (input.type() == typeid(float)) {
            return removeTrailingZeros(std::to_string(std::any_cast<float>(input)));
        
        } else {
            std::string ret = "Unsupported type!";
            ret = ret + " Type:: " + demangle(input.type().name());
            return ret;
        }
    } catch (const std::bad_any_cast& e) {
        std::cerr << "Bad any cast: " << e.what() << std::endl;
        return "Bad any cast!";
    }
}





class ObjectBundler {
public:
    // Bundles the library.
    // The format is:
    // [4 bytes magic "TNDR4"]
    // [4 bytes uint32_t: metadata blob size]
    // [metadata blob: number of pairs, then for each: key length, key, value length, value]
    // [object file content]
    bool bundle(const std::string &objFilePath,
                const std::unordered_map<std::string, std::string> &metadata,
                const std::string &outputFilePath)
    {

        std::ifstream objFile(objFilePath, std::ios::binary);
        if (!objFile) {
            std::cerr << "Error opening object file: " << objFilePath << "\n";
            return false;
        }
        std::vector<char> objContent((std::istreambuf_iterator<char>(objFile)),
                                      std::istreambuf_iterator<char>());
        objFile.close();

        std::stringstream metaStream(std::ios::in | std::ios::out | std::ios::binary);
        // no. of keyvalue pairs
        uint32_t pairCount = static_cast<uint32_t>(metadata.size());
        writeUint32(metaStream, pairCount);

        for (const auto &pair : metadata) {
            writeString(metaStream, pair.first);
            writeString(metaStream, pair.second);
        }
        std::string metaDataBlob = metaStream.str();
        uint32_t metaBlobSize = static_cast<uint32_t>(metaDataBlob.size());



        std::ofstream outFile(outputFilePath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputFilePath << "\n";
            return false;
        }
        // Magic bytes here.
        outFile.write("TNDR4", 5);

        writeUint32(outFile, metaBlobSize);

        outFile.write(metaDataBlob.data(), metaBlobSize);

        if (!objContent.empty()) {
            outFile.write(objContent.data(), objContent.size());
        }
        outFile.close();
        return true;
    }

    // Unbundler
    // It reads the header, extracts the metadata blob (parsing the key-value pairs)
    // and then reads the remaining bytes as the object file content.
    bool unbundle(const std::string &bundleFilePath,
                  std::unordered_map<std::string, std::string>* pmetadata,
                  std::vector<char> *pobjContent)
    {   
        std::unordered_map<std::string, std::string> metadata;
        std::vector<char> objContent;
        std::ifstream inFile(bundleFilePath, std::ios::binary);
        if (!inFile) {
            std::cerr << "Error opening bundle file: " << bundleFilePath << "\n";
            return false;
        }
        char magic[5];
        inFile.read(magic, 5);
        if (std::string(magic, 5) != "TNDR4") {
            std::cerr << "Invalid bundle file format\n";
            return false;
        }
        // Read blob size.
        uint32_t metaBlobSize = readUint32(inFile);

        std::vector<char> metaBuffer(metaBlobSize);
        inFile.read(metaBuffer.data(), metaBlobSize);
        std::stringstream metaStream(std::string(metaBuffer.begin(), metaBuffer.end()),
                                     std::ios::in | std::ios::binary);

        uint32_t pairCount = readUint32(metaStream);
        for (uint32_t i = 0; i < pairCount; i++) {
            std::string key = readString(metaStream);
            std::string value = readString(metaStream);
            metadata[key] = value;
        }

        objContent.assign((std::istreambuf_iterator<char>(inFile)),
                           std::istreambuf_iterator<char>());
        inFile.close();
        *pmetadata = metadata;
        *pobjContent = objContent;
        return true;
    }

private:
    // Helpers
    void writeUint32(std::ostream &os, uint32_t value) {
        os.write(reinterpret_cast<const char *>(&value), sizeof(value));
    }
    uint32_t readUint32(std::istream &is) {
        uint32_t value;
        is.read(reinterpret_cast<char *>(&value), sizeof(value));
        return value;
    }

    void writeString(std::ostream &os, const std::string &str) {
        uint32_t len = static_cast<uint32_t>(str.size());
        writeUint32(os, len);
        os.write(str.data(), len);
    }

    std::string readString(std::istream &is) {
        uint32_t len = readUint32(is);
        std::string str(len, '\0');
        is.read(&str[0], len);
        return str;
    }
};

// more converters


std::any convertToInt8(const std::any& value) {
    
    if (value.type() == typeid(int32_t)) {
        return static_cast<int8_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<int8_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<int8_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<int8_t>(std::any_cast<double>(value));
    } else {
        throw std::bad_any_cast();
    }
}

std::any convertToInt16(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<int16_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<int16_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<int16_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<int16_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<int16_t>(std::any_cast<double>(value));
    } else {
        throw std::bad_any_cast();
    }
}

std::any convertToInt32(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<int32_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<int32_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<int32_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<int32_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<int32_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<int32_t>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<int32_t>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<int32_t>(std::any_cast<double>(value));
    } else {
        //println(value.type().name());
        throw std::bad_any_cast();
    }
}

std::any convertToInt64(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<int64_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<int64_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<int64_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<int64_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<int64_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<int64_t>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<int64_t>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<int64_t>(std::any_cast<double>(value));
    } else {
        throw std::bad_any_cast();
    }
}

std::any convertToUInt8(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<uint8_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<uint8_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<uint8_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<uint8_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<uint8_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<uint8_t>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<uint8_t>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<uint8_t>(std::any_cast<double>(value));
    } else {
        //println(demangle(value.type().name()));
        throw std::bad_any_cast();
    }
}

std::any convertToUInt16(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<uint16_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<uint16_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<uint16_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<uint16_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<uint16_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<uint16_t>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<uint16_t>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<uint16_t>(std::any_cast<double>(value));
    } else {
        throw std::bad_any_cast();
    }
}

std::any convertToUInt32(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<uint32_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<uint32_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<uint32_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<uint32_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<uint32_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<uint32_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<uint32_t>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<uint32_t>(std::any_cast<double>(value));
    } else {
        throw std::bad_any_cast();
    }
}

std::any convertToUInt64(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<uint64_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<uint64_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<uint64_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<uint64_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<uint64_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<uint64_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<uint64_t>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<uint64_t>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(uintptr_t)) {
        return static_cast<uint64_t>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<uint64_t>(std::any_cast<double>(value));
    } else {
        
        throw std::bad_any_cast();
    }
}

std::any convertToFloat(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<float>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<float>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<float>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<float>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<float>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<float>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<float>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<float>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(float)) {
        return std::any_cast<float>(value);
    } else if (value.type() == typeid(double)) {
        return static_cast<float>(std::any_cast<double>(value));
    } else {
        throw std::bad_any_cast();
    }
}

std::any convertToDouble(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<double>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<double>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<double>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<double>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<double>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<double>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<double>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<double>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(float)) {
        return static_cast<double>(std::any_cast<float>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<double>(std::any_cast<double>(value));
    } else if (value.type() == typeid(ssize_t)) {
        return static_cast<double>(std::any_cast<float>(value));
    } else if (value.type() == typeid(size_t)) {
        return static_cast<double>(std::any_cast<double>(value));
    } else if (value.type() == typeid(std::string)) {
        return std::stod(std::any_cast<std::string>(value));
    } else {
        //println(demangle(value.type().name()));
        throw std::bad_any_cast();
    }
}

std::any convertToStr(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return std::to_string(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return std::to_string(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return std::to_string(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return std::to_string(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return std::to_string(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return std::to_string(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return std::to_string(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return std::to_string(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(float)) {
        return std::to_string(std::any_cast<float>(value));
    } else if (value.type() == typeid(double)) {
        return std::to_string(std::any_cast<double>(value));
    } else if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value);
    } else {
        throw std::bad_any_cast();
    }
}

std::any convertToChar(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<char>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<char>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<char>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<char>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<char>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<char>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<char>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<char>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(float)) {
        return static_cast<char>(std::any_cast<float>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<char>(static_cast<int>(std::any_cast<double>(value)));
    } else if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value)[0];
    } else {
        throw std::bad_any_cast();
    }
}

std::any convertToWChar(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<wchar_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<wchar_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<wchar_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<wchar_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<wchar_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<wchar_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<wchar_t>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<wchar_t>(std::any_cast<uint64_t>(value));
    } else if (value.type() == typeid(float)) {
        return static_cast<wchar_t>(std::any_cast<float>(value));
    } else if (value.type() == typeid(double)) {
        return static_cast<wchar_t>(std::any_cast<double>(value));
    } else if (value.type() == typeid(std::string)) {
        return static_cast<wchar_t>(std::any_cast<std::string>(value)[0]);
    } else {
        throw std::bad_any_cast();
    }
}

ssize_t convertToSSize(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        return static_cast<ssize_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        return static_cast<ssize_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        return static_cast<ssize_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        return static_cast<ssize_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<ssize_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<ssize_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<ssize_t>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        // Prevent casting large unsigned values to signed ssize_t
        if (std::any_cast<uint64_t>(value) > static_cast<uint64_t>(std::numeric_limits<ssize_t>::max())) {
            throw std::overflow_error("Value too large to fit into ssize_t");
        }
        return static_cast<ssize_t>(std::any_cast<uint64_t>(value));
    } else {
        throw std::bad_any_cast();
    }
}

// Helper function for size_t conversion (unsigned size type)
size_t convertToSize(const std::any& value) {
    if (value.type() == typeid(int8_t)) {
        if (std::any_cast<int8_t>(value) < 0) throw std::overflow_error("Negative value cannot fit in unsigned size_t");
        return static_cast<size_t>(std::any_cast<int8_t>(value));
    } else if (value.type() == typeid(int16_t)) {
        if (std::any_cast<int16_t>(value) < 0) throw std::overflow_error("Negative value cannot fit in unsigned size_t");
        return static_cast<size_t>(std::any_cast<int16_t>(value));
    } else if (value.type() == typeid(int32_t)) {
        if (std::any_cast<int32_t>(value) < 0) throw std::overflow_error("Negative value cannot fit in unsigned size_t");
        return static_cast<size_t>(std::any_cast<int32_t>(value));
    } else if (value.type() == typeid(int64_t)) {
        if (std::any_cast<int64_t>(value) < 0) throw std::overflow_error("Negative value cannot fit in unsigned size_t");
        return static_cast<size_t>(std::any_cast<int64_t>(value));
    } else if (value.type() == typeid(uint8_t)) {
        return static_cast<size_t>(std::any_cast<uint8_t>(value));
    } else if (value.type() == typeid(uint16_t)) {
        return static_cast<size_t>(std::any_cast<uint16_t>(value));
    } else if (value.type() == typeid(uint32_t)) {
        return static_cast<size_t>(std::any_cast<uint32_t>(value));
    } else if (value.type() == typeid(uint64_t)) {
        return static_cast<size_t>(std::any_cast<uint64_t>(value));
    } else {
        throw std::bad_any_cast();
    }
}

// Map of converters
tsl::ordered_map<std::string, std::function<std::any(const std::any&)>> converters = {
    {"i8", convertToInt8},
    {"i16", convertToInt16},
    {"i32", convertToInt32},
    {"i64", convertToInt64},
    {"u8", convertToUInt8},
    {"u16", convertToUInt16},
    {"u32", convertToUInt32},
    {"u64", convertToUInt64},
    {"f32", convertToFloat},
    {"f64", convertToDouble},
    {"RawString", convertToString},
    {"Character", convertToChar},
    {"wchar", convertToWChar}, // needs patching
    {"isize", convertToSSize},
    {"usize", convertToSize}
};
// various value structs
struct nullification {

        bool operator==(const nullification& other) const {
        return (*this == other);
    }

    // Overloading the != operator as a member function
    bool operator!=(const nullification& other) const {
        return !(*this == other);  // reuse the == operator
    }
};

struct nullificationptr {

        bool operator==(const nullification& other) const {
        return (*this == other);
    }

    // Overloading the != operator as a member function
    bool operator!=(const nullification& other) const {
        return !(*this == other);  // reuse the == operator
    }
};
struct NoAssign {

        bool operator==(const nullification& other) const {
        return (*this == other);
    }

    // Overloading the != operator as a member function
    bool operator!=(const nullification& other) const {
        return !(*this == other);  // reuse the == operator
    }
};

struct nullificationv {

        bool operator==(const nullification& other) const {
        return (*this == other);
    }

    // Overloading the != operator as a member function
    bool operator!=(const nullification& other) const {
        return !(*this == other);  // reuse the == operator
    }
};

tsl::ordered_map<std::string,std::shared_ptr<Instruction>> dummyinstructions = {{"none",std::make_shared<resource_instruction>("none",nullptr)},{"null",std::make_shared<resource_instruction>("Null",nullptr)}};
 // primitive types
tsl::ordered_map<std::string, std::any> dummyValues = {
        {"i8", static_cast<std::any>(static_cast<int8_t>(0))}, // i8
        {"i16", static_cast<std::any>(static_cast<int16_t>(0))}, // i16
        {"i32", static_cast<std::any>(static_cast<int32_t>(0))}, // i32
        {"i64", static_cast<std::any>(static_cast<int64_t>(0))}, // i64
        {"u8", static_cast<std::any>(static_cast<uint8_t>(0))}, // u8
        {"u16", static_cast<std::any>(static_cast<uint16_t>(0))}, // u16
        {"u32", static_cast<std::any>(static_cast<uint32_t>(0))}, // u32
        {"u64", static_cast<std::any>(static_cast<uint64_t>(0))}, // u64
        {"f32", static_cast<std::any>(0.0f)}, // f32
        {"f64", static_cast<std::any>(0.0)} , // f64
        {"RawString", std::any(std::string("example string"))}, // String (C style)
        
        {"Character", std::any('A')}, // char
        {"Bool",std::any(true)},
        {"nil",std::any(nullificationv{})}
        //{"wchar", std::any(L'あ')} // wchar (Japanese character lol)
};
#endif