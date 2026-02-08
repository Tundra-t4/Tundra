#ifndef BASE_HPP
#define BASE_HPP

// Standard library includes
#include <iostream>
#include <any>
#include <fstream>
#include <string_view>
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
#include <map>
#include <typeindex>
#include <optional>
#include <atomic>


#include "include/ordered_map.h"
#include "modules/HParser/Hparser.h"
#include "../polaroid/polaroid.hpp"
#include "include/sjson.hpp"

// Forward declarations
class SymbolTable;
class Scope;
class ASTNode;

// ============================================================================
// CONSTANTS & STYLING
// ============================================================================

namespace Colors {
    std::string RED = "\033[31m";
    std::string GREEN = "\033[32m";
    std::string YELLOW = "\x1b[33m";
    std::string CYAN = "\033[96m";
    std::string LIGHT_BLUE = "\033[94m";
    std::string BOLD = "\033[1m";
    std::string RESET = "\033[0m";
}

namespace Symbols {
    std::string TOP_CORNER = "┏";
    std::string BOTTOM_CORNER = "┗";
    std::string TOP_ARC = "╭";
    std::string BOTTOM_ARC = "└▶";
    std::string ARC_DOWN = "│";
    std::string LINE_DOWN = "┣";
    std::string ARROW = "⟶";
    std::string DOUBLE_ARROW = "⇒";
    std::string FILLED_CIRCLE = "●";
    std::string CIRCLE = "◯";
}

// ============================================================================
// RESOURCE MANAGEMENT
// ============================================================================

// Reference-counted resource manager
template<typename T>
class ResourceHandle {
private:
    struct RefCountedResource {
        T resource;
        std::atomic<size_t> ref_count;
        
        explicit RefCountedResource(T&& res) 
            : resource(std::move(res)), ref_count(1) {}
    };
    
    RefCountedResource* ptr_ = nullptr;
    
    void acquire() {
        if (ptr_) {
            ptr_->ref_count.fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    void release() {
        if (ptr_ && ptr_->ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete ptr_;
            ptr_ = nullptr;
        }
    }
    
public:
    ResourceHandle() = default;
    
    explicit ResourceHandle(T&& resource) 
        : ptr_(new RefCountedResource(std::move(resource))) {}
    
    ResourceHandle(const ResourceHandle& other) : ptr_(other.ptr_) {
        acquire();
    }
    
    ResourceHandle(ResourceHandle&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    
    ResourceHandle& operator=(const ResourceHandle& other) {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            acquire();
        }
        return *this;
    }
    
    ResourceHandle& operator=(ResourceHandle&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
    
    ~ResourceHandle() {
        release();
    }
    
    T* get() { return ptr_ ? &ptr_->resource : nullptr; }
    const T* get() const { return ptr_ ? &ptr_->resource : nullptr; }
    
    T& operator*() { return ptr_->resource; }
    const T& operator*() const { return ptr_->resource; }
    
    T* operator->() { return &ptr_->resource; }
    const T* operator->() const { return &ptr_->resource; }
    
    explicit operator bool() const { return ptr_ != nullptr; }
    
    size_t use_count() const { 
        return ptr_ ? ptr_->ref_count.load(std::memory_order_relaxed) : 0; 
    }
};

// ============================================================================
// COMPILER STATE - Centralized instead of global variables
// ============================================================================

struct CompilerState {
    // Flags
    bool debug_mode = true;
    bool compile_mode = true;
    bool auto_save_debug = true;
    bool invoking_macro = false;
    
    // Counters
    int warnings_count = 0;
    
    // Configuration
    std::string mangle_rule = "t4";
    std::string typing_rule = "static";
    
    // Collections
    tsl::ordered_map<std::string, std::vector<char>> objects;
    tsl::ordered_map<int, bool> unsafe_declarations;
    tsl::ordered_map<int, bool> safe_declarations;
    tsl::ordered_map<std::string, std::string> pointer_targets;
    tsl::ordered_map<std::string, int> identifiers;
    tsl::ordered_map<std::string, std::vector<std::string>> notes;
    //tsl::ordered_map<int, std::shared_ptr<type_instruction>> forward_types;
    tsl::ordered_map<int, std::any> forward_complex;
    std::unordered_map<std::string, std::string> exports;
    
    std::vector<int> used_ids;
    std::vector<int> warned_ids;
    std::vector<int> bad_node_ids;
    std::vector<int> signal_ids;
    
    // Logging
    std::string log_content;
    
    // Singleton accessor
    static CompilerState& instance() {
        static CompilerState state;
        return state;
    }
    
private:
    CompilerState() = default;
    CompilerState(const CompilerState&) = delete;
    CompilerState& operator=(const CompilerState&) = delete;
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Cantor Pairing function (optimized)
inline uint64_t cantor_pair(uint64_t x, uint64_t y) noexcept {
    if (x > y) std::swap(x, y);
    return ((x + y) * (x + y + 1)) / 2 + y;
}

// Logging functions
inline void logat(std::string content, std::string source) {
    auto& state = CompilerState::instance();
    if (state.debug_mode) {
        state.log_content += "@";
        state.log_content += source;
        state.log_content += " -> ";
        state.log_content += content;
        state.log_content += "\n";
        
        if (state.auto_save_debug) {
            std::ofstream log_file("Tundra.log");
            log_file << state.log_content << "Saved log!";
        }
    }
}

// Misc util fns

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





// String utilities (using string for efficiency)
inline std::vector<std::string> split(std::string str, char delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        result.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    result.emplace_back(str.substr(start));
    return result;
}

inline std::string replace_all(std::string str, std::string from, std::string to) {
    if (from.empty()) return str;
    
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
    return str;
}

inline std::string replace(std::string str, std::string from, std::string to) {
    if (from.empty()) return str;
    
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
    return str;
}

void println(std::string str){
    std::cout << str << std::endl;
}

// Number parsing utilities
inline uint64_t parse_hex(std::string hex_str) {
    uint64_t result = 0;
    std::stringstream ss;
    ss << std::hex << hex_str;
    ss >> result;
    return result;
}

inline uint64_t parse_binary(std::string_view bin_str) {
    // Remove "0b" prefix if present
    if (bin_str.starts_with("0b")) {
        bin_str.remove_prefix(2);
    }
    
    uint64_t result = 0;
    for (char c : bin_str) {
        result = (result << 1) | (c - '0');
    }
    return result;
}

inline uint64_t parse_octal(std::string_view oct_str) {
    // Remove "0o" prefix if present
    if (oct_str.starts_with("0o")) {
        oct_str.remove_prefix(2);
    }
    
    uint64_t result = 0;
    for (char c : oct_str) {
        result = (result << 3) | (c - '0');
    }
    return result;
}

int64_t safe_string_to_int64(std::string str) {
    try {
        size_t pos;
        std::string str_copy(str);
        int64_t value = std::stoll(str_copy, &pos);
        
        if (pos != str.length()) {
            return 0;
        }
        return value;
    } catch (...) {
        return 0;
    }
}

// Collection utilities
template<typename T>
inline bool contains(const std::vector<T>& vec, const T& value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

template<typename T, typename K>
inline T reverse_find(tsl::ordered_map<T, K>& map, const K& value,bool& checker) {
    checker=true;
    for (const auto& elem : map) {
        if (elem.second == value) {
            return elem.first;
        }
    }
    checker=false;
}

// shared_ptr casting
template<typename T, typename K>
std::shared_ptr<T> SCAST(std::shared_ptr<K> ptr) {
    return std::dynamic_pointer_cast<T>(ptr);
}

// Debug output
template<typename T>
inline std::string vector_to_string(const std::vector<T>& vec) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i != vec.size() - 1) {
            oss << ", ";
        }
    }
    return oss.str();
}

// Type name demangling
inline std::string demangle(const char* name) {
    int status = 0;
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled) ? demangled : name;
    free(demangled);
    return result;
}

tsl::ordered_map<std::string, std::string> gerr(std::vector<std::string> g){
    tsl::ordered_map<std::string, std::string> x;
    for (size_t i = 0; i < g.size(); ++i) {
        x[g[i]] = g[i+1];
        ++i;
    }
    return x;
}

// ============================================================================
// TYPE CONVERSION SYSTEM (Optimized with templates)
// ============================================================================

// Template-based conversion system for better type safety and performance
template<typename To, typename From>
inline To numeric_cast(From value) {
    if (std::is_signed_v<To> && std::is_unsigned_v<From>) {
        if (value > static_cast<From>(std::numeric_limits<To>::max())) {
            throw std::overflow_error("Value too large for target type");
        }
    } else if (std::is_unsigned_v<To> && std::is_signed_v<From>) {
        if (value < 0) {
            throw std::overflow_error("Negative value cannot fit in unsigned type");
        }
    }
    return static_cast<To>(value);
}



// Type conversion functions using visitor pattern
class TypeConverter {
public:
    template<typename T>
    static std::any convert_to(const std::any& value) {
        if (value.type() == typeid(int8_t)) {
            return numeric_cast<T>(std::any_cast<int8_t>(value));
        } else if (value.type() == typeid(int16_t)) {
            return numeric_cast<T>(std::any_cast<int16_t>(value));
        } else if (value.type() == typeid(int32_t)) {
            return numeric_cast<T>(std::any_cast<int32_t>(value));
        } else if (value.type() == typeid(int64_t)) {
            return numeric_cast<T>(std::any_cast<int64_t>(value));
        } else if (value.type() == typeid(uint8_t)) {
            return numeric_cast<T>(std::any_cast<uint8_t>(value));
        } else if (value.type() == typeid(uint16_t)) {
            return numeric_cast<T>(std::any_cast<uint16_t>(value));
        } else if (value.type() == typeid(uint32_t)) {
            return numeric_cast<T>(std::any_cast<uint32_t>(value));
        } else if (value.type() == typeid(uint64_t)) {
            return numeric_cast<T>(std::any_cast<uint64_t>(value));
        } else if (value.type() == typeid(float)) {
            return numeric_cast<T>(std::any_cast<float>(value));
        } else if (value.type() == typeid(double)) {
            return numeric_cast<T>(std::any_cast<double>(value));
        } else if (value.type() == typeid(std::string)) {
            if (std::is_same_v<T, std::string>) {
                return value;
            } else if (std::is_arithmetic_v<T>) {
                return numeric_cast<T>(std::stod(std::any_cast<std::string>(value)));
            }
        }
        throw std::bad_any_cast();
    }
    
    static std::string to_string(const std::any& value) {
        if (value.type() == typeid(std::string)) {
            return std::any_cast<std::string>(value);
        } else if (value.type() == typeid(int8_t)) {
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
        }
        throw std::bad_any_cast();
    }
};

// Converter map
inline const tsl::ordered_map<std::string, std::function<std::any(const std::any&)>>& get_converters() {
    static tsl::ordered_map<std::string, std::function<std::any(const std::any&)>> converters = {
        {"i8", [](const std::any& v) { return TypeConverter::convert_to<int8_t>(v); }},
        {"i16", [](const std::any& v) { return TypeConverter::convert_to<int16_t>(v); }},
        {"i32", [](const std::any& v) { return TypeConverter::convert_to<int32_t>(v); }},
        {"i64", [](const std::any& v) { return TypeConverter::convert_to<int64_t>(v); }},
        {"u8", [](const std::any& v) { return TypeConverter::convert_to<uint8_t>(v); }},
        {"u16", [](const std::any& v) { return TypeConverter::convert_to<uint16_t>(v); }},
        {"u32", [](const std::any& v) { return TypeConverter::convert_to<uint32_t>(v); }},
        {"u64", [](const std::any& v) { return TypeConverter::convert_to<uint64_t>(v); }},
        {"f32", [](const std::any& v) { return TypeConverter::convert_to<float>(v); }},
        {"f64", [](const std::any& v) { return TypeConverter::convert_to<double>(v); }},
        {"RawString", TypeConverter::to_string},
        {"Character", [](const std::any& v) { return TypeConverter::convert_to<char>(v); }},
    };
    return converters;
}

// ============================================================================
// SPECIAL VALUE TYPES
// ============================================================================

struct nullification {
    bool operator==(const nullification&) const { return true; }
    bool operator!=(const nullification&) const { return false; }
};

struct nullificationptr {
    bool operator==(const nullificationptr&) const { return true; }
    bool operator!=(const nullificationptr&) const { return false; }
};

struct NoAssign {
    bool operator==(const NoAssign&) const { return true; }
    bool operator!=(const NoAssign&) const { return false; }
};

struct nullificationv {
    bool operator==(const nullificationv&) const { return true; }
    bool operator!=(const nullificationv&) const { return false; }
};

// ============================================================================
// PRIMITIVE TYPE DEFAULTS
// ============================================================================

inline const tsl::ordered_map<std::string, std::any>& get_dummy_values() {
    static tsl::ordered_map<std::string, std::any> values = {
        {"i8", std::any(static_cast<int8_t>(0))},
        {"i16", std::any(static_cast<int16_t>(0))},
        {"i32", std::any(static_cast<int32_t>(0))},
        {"i64", std::any(static_cast<int64_t>(0))},
        {"u8", std::any(static_cast<uint8_t>(0))},
        {"u16", std::any(static_cast<uint16_t>(0))},
        {"u32", std::any(static_cast<uint32_t>(0))},
        {"u64", std::any(static_cast<uint64_t>(0))},
        {"f32", std::any(0.0f)},
        {"f64", std::any(0.0)},
        {"RawString", std::any(std::string(""))},
        {"Character", std::any('A')},
        {"Bool", std::any(true)},
        {"nil", std::any(nullificationv{})}
    };
    return values;
}

// ArgParser
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
    



// ============================================================================
// GLOBAL STATE ACCESSORS (for backward compatibility)
// ============================================================================

// These provide backward-compatible access to the centralized CompilerState
inline auto& debug = CompilerState::instance().debug_mode;
inline auto& compile = CompilerState::instance().compile_mode;
inline auto& compileflag = CompilerState::instance().compile_mode;
inline auto& autosavedebug = CompilerState::instance().auto_save_debug;
inline auto& invokingMacro = CompilerState::instance().invoking_macro;
inline auto& warns = CompilerState::instance().warnings_count;
inline auto& mangle_rule = CompilerState::instance().mangle_rule;
inline auto& logcon = CompilerState::instance().log_content;
inline auto& objects = CompilerState::instance().objects;
inline auto& isunsafedecl = CompilerState::instance().unsafe_declarations;
inline auto& issafedecl = CompilerState::instance().safe_declarations;
inline auto& ptrnowptsto = CompilerState::instance().pointer_targets;
inline auto& ids = CompilerState::instance().identifiers;
inline auto& notes = CompilerState::instance().notes;
//inline auto& forwards = CompilerState::instance().forward_types;
inline auto& forwardscplex = CompilerState::instance().forward_complex;
inline auto& exports = CompilerState::instance().exports;
inline auto& isused = CompilerState::instance().used_ids;
inline auto& hasbeenwarned = CompilerState::instance().warned_ids;
inline auto& isbadnode = CompilerState::instance().bad_node_ids;
inline auto& issig = CompilerState::instance().signal_ids;

// Backward compatibility for converters and dummy values
inline const auto& converters = get_converters();
inline const auto& dummyValues = get_dummy_values();

 
tsl::ordered_map<std::string,std::string> ruleset;
std::vector<std::string> allowances;
tsl::ordered_map<std::string,std::any> macroinfo;

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
std::string linkbottom = hashint ?( Symbols::BOTTOM_ARC + " ") : "";
std::string linktop = hashint ? (Symbols::TOP_ARC + " ") : "";
std::string linkblock = hashint ? (Symbols::ARC_DOWN + " ") : (ishint ? "  " :"");
auto chosenblock = Colors::RED;
if (warning){
idr = " WARNING: ";

}
if (ishint){
idr = Colors::BOLD + " " + hint[0] + " " + Colors::RESET ;
chosenblock = Colors::CYAN;

}


std::cout << Colors::BOLD << (ishint ? Symbols::ARC_DOWN + " " : linktop) << (!warning ? (chosenblock + Symbols::FILLED_CIRCLE + Colors::RESET) : (Colors::YELLOW + Symbols::CIRCLE + Colors::RESET)) <<" " << Colors::RESET << filepath << " [" + std::to_string(errorLine)  + ":" + std::to_string(startPos) + ".." + std::to_string(endPos) + "]\n";

if (ishint){
std::cout << Colors::BOLD << Symbols::BOTTOM_ARC << Colors::RESET << "  " << hint[1] << std::endl;
}


if (!beforeLine.empty() && errorLine-1 > 0) {
std::cout << linkblock << Colors::LIGHT_BLUE << errorLine - 1 << Colors::RESET << std::string(4-std::to_string(errorLine-1).size(), ' ') + "║     " << beforeLine << "\n";
}
std::cout << linkblock << Colors::LIGHT_BLUE << errorLine << Colors::RESET << std::string(4-std::to_string(errorLine).size(), ' ') + "║     " << errorLineContent << "\n";


if (startPos > endPos){
endPos = errorLineContent.size();
}
std::cout << linkblock << std::string(10 + (startPos == 0 ? 0 : 0), ' ') << std::string(startPos, ' ') << Colors::RED << std::string(endPos - startPos, (ishint ? '-': '^')) << Colors::RESET << (ishint ? ">": "") << idr << error << "\n";

if (!afterLine.empty()) {
std::cout << linkblock << Colors::LIGHT_BLUE << errorLine + 1 << Colors::RESET << std::string(4-std::to_string(errorLine+1).size(), ' ') + + "║     " << afterLine << "\n";
}
if (notes.find("reason") != notes.end()){
std::cout << linkblock << std::string(4, ' ') + "-  " << Colors::BOLD << "reason" << ": " << Colors::RESET << notes["reason"] << "\n";
notes.erase("reason");
}
if (notes.find("note") != notes.end()){
std::cout << linkblock << std::string(4, ' ') + "-  " << Colors::BOLD << "note" << ": " << Colors::RESET << notes["note"] << "\n";
notes.erase("note");
}
if (notes.find("[X]") != notes.end()){
std::cout << linkblock << std::string(4, ' ') + "=  " << Colors::BOLD << "Example" << ": " << Colors::RESET << "\n";
std::cout << linkblock << std::string(4, ' ') + "=  " << Colors::BOLD << "[X]" << ": " << Colors::RESET << notes["[X]"] << "\n";
notes.erase("[X]");
std::cout << linkblock << std::string(4, ' ') + "=  " << Colors::BOLD << "[O]" << ": " << Colors::RESET << notes["[O]"] << "\n";
notes.erase("[O]");
}

for (const auto& [key, value] : notes) {
std::cout << linkblock << std::string(4, ' ') + "~  " << Colors::BOLD << key << ": " << Colors::RESET << value << "\n";

}



if (invokingMacro && !warning){
std::cout << linkblock << std::endl;
auto lposmacro = std::any_cast<std::vector<int>>(macroinfo["errorpos"]);
invokingMacro = false;
displayError(std::any_cast<std::string>(macroinfo["content"]) ,lposmacro[0],lposmacro[1],lposmacro[2],{},true,{"hint:",std::any_cast<std::string>(macroinfo["hint"])});
}
if (!warning){
std::cout <<  Colors::RED + Symbols::FILLED_CIRCLE + Colors::RESET + " Build failed." << std::endl;
exit(0);
} else {
std::cout << std::endl;
}

} catch (const std::exception& e) {
std::cout << Colors::RED << "Failed to display error: '" << error << "' " << e.what() << Colors::RESET << "\n";
exit(0);
}
}

#endif // BASE_HPP