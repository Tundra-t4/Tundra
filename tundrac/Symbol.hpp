#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
#include <memory>
#include <any>
#include <vector>
#include <optional>

// Forward declarations
class Type;
class ASTNode;

// Symbol kinds
enum class SymbolKind {
    VARIABLE,
    FUNCTION,
    PARAMETER,
    TYPE,
    STRUCT,
    ENUM,
    CLASS,
    MODULE,
    CONSTANT
};

// Symbol attributes/flags
enum class SymbolFlags {
    NONE = 0,
    MUTABLE = 1 << 0,
    CONST = 1 << 1,
    PUBLIC = 1 << 2,
    PRIVATE = 1 << 3,
    EXTERNAL = 1 << 4,
    UNSAFE = 1 << 5,
    BORROWED = 1 << 6,
    OWNED = 1 << 7
};

inline SymbolFlags operator|(SymbolFlags a, SymbolFlags b) {
    return static_cast<SymbolFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline SymbolFlags operator&(SymbolFlags a, SymbolFlags b) {
    return static_cast<SymbolFlags>(static_cast<int>(a) & static_cast<int>(b));
}

inline bool has_flag(SymbolFlags flags, SymbolFlags check) {
    return (flags & check) != SymbolFlags::NONE;
}

// Symbol class - represents an entry in the symbol table
class Symbol {
public:
    Symbol(const std::string& name, SymbolKind kind, std::shared_ptr<Type> type,
           std::shared_ptr<ASTNode> declaration = nullptr, SymbolFlags flags = SymbolFlags::NONE)
        : name_(name)
        , kind_(kind)
        , type_(type)
        , declaration_(declaration)
        , flags_(flags)
        , unique_id_(next_id_++)
        , ref_count_(0)
        , scope_depth_(0)
    {}

    // Getters
    const std::string& name() const { return name_; }
    SymbolKind kind() const { return kind_; }
    std::shared_ptr<Type> type() const { return type_; }
    std::shared_ptr<ASTNode> declaration() const { return declaration_; }
    SymbolFlags flags() const { return flags_; }
    size_t unique_id() const { return unique_id_; }
    size_t ref_count() const { return ref_count_; }
    size_t scope_depth() const { return scope_depth_; }

    // Flag checking
    bool is_mutable() const { return has_flag(flags_, SymbolFlags::MUTABLE); }
    bool is_const() const { return has_flag(flags_, SymbolFlags::CONST); }
    bool is_public() const { return has_flag(flags_, SymbolFlags::PUBLIC); }
    bool is_external() const { return has_flag(flags_, SymbolFlags::EXTERNAL); }
    bool is_unsafe() const { return has_flag(flags_, SymbolFlags::UNSAFE); }
    bool is_borrowed() const { return has_flag(flags_, SymbolFlags::BORROWED); }
    bool is_owned() const { return has_flag(flags_, SymbolFlags::OWNED); }

    // Setters
    void set_type(std::shared_ptr<Type> type) { type_ = type; }
    void set_flags(SymbolFlags flags) { flags_ = flags; }
    void add_flag(SymbolFlags flag) { flags_ = flags_ | flag; }
    void set_scope_depth(size_t depth) { scope_depth_ = depth; }
    
    // Reference counting for lifetime tracking
    void add_reference() { ++ref_count_; }
    void remove_reference() { 
        if (ref_count_ > 0) --ref_count_; 
    }
    bool has_references() const { return ref_count_ > 0; }

    // Type checking helpers
    bool is_variable() const { return kind_ == SymbolKind::VARIABLE; }
    bool is_function() const { return kind_ == SymbolKind::FUNCTION; }
    bool is_type() const { return kind_ == SymbolKind::TYPE; }
    bool is_parameter() const { return kind_ == SymbolKind::PARAMETER; }

    // Display
    std::string to_string() const;

private:
    std::string name_;
    SymbolKind kind_;
    std::shared_ptr<Type> type_;
    std::shared_ptr<ASTNode> declaration_;
    SymbolFlags flags_;
    size_t unique_id_;
    size_t ref_count_;
    size_t scope_depth_;
    
    static size_t next_id_;
};

// Initialize static member
inline size_t Symbol::next_id_ = 0;

inline std::string Symbol::to_string() const {
    std::string result = "Symbol{name: " + name_ + ", kind: ";
    
    switch (kind_) {
        case SymbolKind::VARIABLE: result += "VARIABLE"; break;
        case SymbolKind::FUNCTION: result += "FUNCTION"; break;
        case SymbolKind::PARAMETER: result += "PARAMETER"; break;
        case SymbolKind::TYPE: result += "TYPE"; break;
        case SymbolKind::STRUCT: result += "STRUCT"; break;
        case SymbolKind::ENUM: result += "ENUM"; break;
        case SymbolKind::CLASS: result += "CLASS"; break;
        case SymbolKind::MODULE: result += "MODULE"; break;
        case SymbolKind::CONSTANT: result += "CONSTANT"; break;
    }
    
    result += ", id: " + std::to_string(unique_id_);
    result += ", refs: " + std::to_string(ref_count_);
    result += "}";
    
    return result;
}

#endif // SYMBOL_HPP